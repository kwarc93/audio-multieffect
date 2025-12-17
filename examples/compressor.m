clear all
close all
clc

pkg load signal
output_precision(16)

% Script generates compressor/limiter coefficients for audio multieffect
% A compressor reduces dynamic range by attenuating signals above a threshold

fs = 48000; % Sample rate

% Compressor parameters
threshold_db = -20;    % Threshold in dB (signals above this are compressed)
ratio = 4;             % Compression ratio (4:1 means 4dB input -> 1dB output above threshold)
knee_db = 6;           % Soft knee width in dB (0 = hard knee)
attack_ms = 5;         % Attack time in milliseconds
release_ms = 100;      % Release time in milliseconds
makeup_gain_db = 10;   % Makeup gain to compensate for compression

% Convert time constants to samples
attack_samples = (attack_ms / 1000) * fs;
release_samples = (release_ms / 1000) * fs;

% Calculate time constants (exponential smoothing coefficients)
% alpha = 1 - exp(-1 / (time_constant_in_samples))
attack_coeff = 1 - exp(-1 / attack_samples);
release_coeff = 1 - exp(-1 / release_samples);

% Convert dB values to linear
threshold_linear = 10^(threshold_db / 20);
makeup_gain_linear = 10^(makeup_gain_db / 20);

% Generate compression curve (input vs output in dB)
input_db = -60:0.1:0;  % Input range from -60dB to 0dB
output_db = zeros(size(input_db));

for i = 1:length(input_db)
    in_db = input_db(i);
    
    if knee_db > 0
        % Soft knee compression
        if in_db < (threshold_db - knee_db/2)
            % Below knee - no compression
            output_db(i) = in_db;
        elseif in_db > (threshold_db + knee_db/2)
            % Above knee - full compression
            output_db(i) = threshold_db + (in_db - threshold_db) / ratio;
        else
            % In knee region - smooth transition
            knee_factor = (in_db - threshold_db + knee_db/2) / knee_db;
            compressed = threshold_db + (in_db - threshold_db) / ratio;
            output_db(i) = in_db + knee_factor * (compressed - in_db);
        end
    else
        % Hard knee compression
        if in_db <= threshold_db
            output_db(i) = in_db;
        else
            output_db(i) = threshold_db + (in_db - threshold_db) / ratio;
        end
    end
end

% Calculate gain reduction curve
gain_reduction_db = output_db - input_db;

% Display compressor information
printf('/* Dynamic Range Compressor */\n');
printf('/* Sample Rate: %d Hz */\n', fs);
printf('/* Threshold: %.1f dB */\n', threshold_db);
printf('/* Ratio: %.1f:1 */\n', ratio);
printf('/* Knee: %.1f dB */\n', knee_db);
printf('/* Attack: %.1f ms (%.0f samples) */\n', attack_ms, attack_samples);
printf('/* Release: %.1f ms (%.0f samples) */\n', release_ms, release_samples);
printf('/* Makeup Gain: %.1f dB */\n\n', makeup_gain_db);

% Generate C++ constants
printf('/* Compressor Parameters */\n');
printf('static constexpr float comp_threshold_db = %.16f;\n', threshold_db);
printf('static constexpr float comp_threshold_linear = %.16f;\n', threshold_linear);
printf('static constexpr float comp_ratio = %.16f;\n', ratio);
printf('static constexpr float comp_knee_db = %.16f;\n', knee_db);
printf('static constexpr float comp_attack_coeff = %.16f;\n', attack_coeff);
printf('static constexpr float comp_release_coeff = %.16f;\n', release_coeff);
printf('static constexpr float comp_makeup_gain_db = %.16f;\n', makeup_gain_db);
printf('static constexpr float comp_makeup_gain_linear = %.16f;\n\n', makeup_gain_linear);

% Generate lookup table for fast dB to linear conversion (optional optimization)
% This can speed up real-time processing
lut_size = 256;
lut_min_db = -60;
lut_max_db = 0;
lut_db_range = lut_max_db - lut_min_db;
lut_db_step = lut_db_range / (lut_size - 1);

db_to_linear_lut = zeros(1, lut_size);
for i = 1:lut_size
    db_val = lut_min_db + (i-1) * lut_db_step;
    db_to_linear_lut(i) = 10^(db_val / 20);
end

printf('/* Lookup table for dB to linear conversion (optional optimization) */\n');
printf('static constexpr uint32_t comp_lut_size = %d;\n', lut_size);
printf('static constexpr float comp_lut_min_db = %.16f;\n', lut_min_db);
printf('static constexpr float comp_lut_max_db = %.16f;\n', lut_max_db);
printf('static constexpr std::array<float, %d> comp_db_to_linear_lut\n{\n', lut_size);

cols = 6;
rows = ceil(lut_size / cols);
i = 1;
while rows > 0
    c = cols;
    if lut_size - i + 1 < c
        c = lut_size - i + 1;
    end
    rows = rows - 1;
    printf('    ');
    while c > 1
        printf('%.16f, ', db_to_linear_lut(i));
        c = c - 1;
        i = i + 1;
    end
    if i <= lut_size
        if i == lut_size
            printf('%.16f\n', db_to_linear_lut(i));
        else
            printf('%.16f,\n', db_to_linear_lut(i));
        end
        i = i + 1;
    end
end
printf('};\n\n');

% Visualize compressor characteristics
figure('Position', [100, 100, 1400, 900]);

% Plot 1: Input/Output Transfer Function
subplot(2, 3, 1);
plot(input_db, output_db, 'b', 'LineWidth', 2); hold on;
plot(input_db, input_db, '--k', 'LineWidth', 1); % Unity gain reference
plot([threshold_db threshold_db], [-60 0], '--r', 'LineWidth', 1); % Threshold line
grid on;
xlabel('Input Level (dB)');
ylabel('Output Level (dB)');
title('Compressor Transfer Function');
legend('Compressed', 'Unity Gain', 'Threshold', 'Location', 'northwest');
axis([-60 0 -60 0]);

% Plot 2: Gain Reduction
subplot(2, 3, 2);
plot(input_db, gain_reduction_db, 'r', 'LineWidth', 2);
grid on;
xlabel('Input Level (dB)');
ylabel('Gain Reduction (dB)');
title('Gain Reduction Curve');
axis([-60 0 -20 0]);

% Plot 3: Attack and Release Envelopes
subplot(2, 3, 3);
time_samples = 0:0.001*fs:0.5*fs; % 500ms
attack_env = 1 - exp(-attack_coeff * time_samples);
release_env = exp(-release_coeff * time_samples);
plot(time_samples/fs*1000, attack_env, 'b', 'LineWidth', 2); hold on;
plot(time_samples/fs*1000, release_env, 'r', 'LineWidth', 2);
grid on;
xlabel('Time (ms)');
ylabel('Envelope Response');
title('Attack and Release Envelopes');
legend('Attack', 'Release', 'Location', 'east');
axis([0 500 0 1.1]);

% Plot 4: Compression Ratio Visualization
subplot(2, 3, 4);
ratios = [1, 2, 4, 8, 20, 100]; % Different compression ratios
colors = {'k', 'b', 'g', 'r', 'm', 'c'};
for r = 1:length(ratios)
    current_ratio = ratios(r);
    output_temp = zeros(size(input_db));
    for i = 1:length(input_db)
        if input_db(i) <= threshold_db
            output_temp(i) = input_db(i);
        else
            output_temp(i) = threshold_db + (input_db(i) - threshold_db) / current_ratio;
        end
    end
    plot(input_db, output_temp, colors{r}, 'LineWidth', 1.5); hold on;
end
plot(input_db, input_db, '--k', 'LineWidth', 1);
grid on;
xlabel('Input Level (dB)');
ylabel('Output Level (dB)');
title('Effect of Different Compression Ratios');
legend('1:1 (bypass)', '2:1', '4:1', '8:1', '20:1', '100:1 (limiter)', 'Location', 'northwest');
axis([-60 0 -60 0]);

% Plot 5: Knee Comparison
subplot(2, 3, 5);
knee_widths = [0, 3, 6, 12]; % Different knee widths
colors = {'r', 'b', 'g', 'm'};
for k = 1:length(knee_widths)
    current_knee = knee_widths(k);
    output_temp = zeros(size(input_db));
    for i = 1:length(input_db)
        in_db = input_db(i);
        if current_knee > 0
            if in_db < (threshold_db - current_knee/2)
                output_temp(i) = in_db;
            elseif in_db > (threshold_db + current_knee/2)
                output_temp(i) = threshold_db + (in_db - threshold_db) / ratio;
            else
                knee_factor = (in_db - threshold_db + current_knee/2) / current_knee;
                compressed = threshold_db + (in_db - threshold_db) / ratio;
                output_temp(i) = in_db + knee_factor * (compressed - in_db);
            end
        else
            if in_db <= threshold_db
                output_temp(i) = in_db;
            else
                output_temp(i) = threshold_db + (in_db - threshold_db) / ratio;
            end
        end
    end
    plot(input_db, output_temp, colors{k}, 'LineWidth', 1.5); hold on;
end
grid on;
xlabel('Input Level (dB)');
ylabel('Output Level (dB)');
title('Hard Knee vs Soft Knee');
legend('Hard (0dB)', 'Soft (3dB)', 'Soft (6dB)', 'Soft (12dB)', 'Location', 'northwest');
axis([-40 0 -40 0]);

% Plot 6: Time Response Example
subplot(2, 3, 6);
% Generate test signal: quiet -> loud -> quiet
test_duration = 2; % seconds
test_samples = test_duration * fs;
t = (0:test_samples-1) / fs;

% Create amplitude envelope: ramp up, sustain, ramp down
ramp_samples = round(0.2 * fs);
sustain_samples = test_samples - 2*ramp_samples;
test_env = [linspace(0.1, 1, ramp_samples), ones(1, sustain_samples), linspace(1, 0.1, ramp_samples)];

% Simulate compressor envelope follower
comp_env = zeros(1, test_samples);
comp_env(1) = test_env(1);
for i = 2:test_samples
    if test_env(i) > comp_env(i-1)
        % Attack
        comp_env(i) = comp_env(i-1) + attack_coeff * (test_env(i) - comp_env(i-1));
    else
        % Release
        comp_env(i) = comp_env(i-1) + release_coeff * (test_env(i) - comp_env(i-1));
    end
end

plot(t*1000, test_env, 'b', 'LineWidth', 1.5); hold on;
plot(t*1000, comp_env, 'r', 'LineWidth', 1.5);
grid on;
xlabel('Time (ms)');
ylabel('Amplitude');
title('Envelope Follower Response');
legend('Input Signal', 'Compressor Envelope', 'Location', 'north');
axis([0 test_duration*1000 0 1.1]);

printf('/* Implementation Notes:\n');
printf(' * \n');
printf(' * 1. Calculate input level in dB: level_db = 20 * log10(abs(input))\n');
printf(' * 2. Determine if signal is above threshold\n');
printf(' * 3. Calculate gain reduction based on ratio and knee\n');
printf(' * 4. Smooth gain changes using attack/release coefficients:\n');
printf(' *    - If gain_reduction increasing (attack): env += attack_coeff * (target - env)\n');
printf(' *    - If gain_reduction decreasing (release): env += release_coeff * (target - env)\n');
printf(' * 5. Apply gain reduction: output = input * gain_linear\n');
printf(' * 6. Apply makeup gain: output *= makeup_gain_linear\n');
printf(' * \n');
printf(' * For efficiency, use RMS or peak detection for level measurement\n');
printf(' * Consider using the lookup table for fast dB conversions\n');
printf(' * \n');
printf(' * Typical settings:\n');
printf(' *   - Gentle compression: ratio 2:1-4:1, soft knee, slow attack/release\n');
printf(' *   - Heavy compression: ratio 8:1-20:1, hard knee, fast attack\n');
printf(' *   - Limiter: ratio 100:1 or higher, hard knee, very fast attack\n');
printf(' */\n');