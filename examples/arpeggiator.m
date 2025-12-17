clear all
close all
clc

pkg load signal
output_precision(16)

% Script generates arpeggiator pattern coefficients for audio multieffect
% An arpeggiator creates rhythmic patterns by cycling through notes in a chord

fs = 48000; % Sample rate
bpm = 120;  % Beats per minute
pattern_length = 16; % Number of steps in the pattern

% Calculate samples per step
beat_duration = 60 / bpm; % Duration of one beat in seconds
step_duration = beat_duration / 4; % 16th note duration
samples_per_step = round(step_duration * fs);

% Define arpeggiator patterns (0 = off, 1-12 = semitone intervals)
% Pattern 1: Up (major triad)
pattern_up = [0, 4, 7, 12, 7, 4, 0, 0, 0, 4, 7, 12, 7, 4, 0, 0];

% Pattern 2: Down (major triad)
pattern_down = [12, 7, 4, 0, 4, 7, 12, 12, 12, 7, 4, 0, 4, 7, 12, 12];

% Pattern 3: Up-Down (major triad)
pattern_updown = [0, 4, 7, 12, 12, 7, 4, 0, 0, 4, 7, 12, 12, 7, 4, 0];

% Pattern 4: Random (major triad)
pattern_random = [0, 7, 4, 12, 0, 4, 12, 7, 4, 0, 7, 12, 4, 7, 0, 12];

% Pattern 5: Octave (root note only)
pattern_octave = [0, 12, 0, 12, 0, 12, 0, 12, 0, 12, 0, 12, 0, 12, 0, 12];

% Generate pitch shift ratios for each semitone (equal temperament)
% ratio = 2^(semitones/12)
semitone_ratios = zeros(1, 13);
for i = 0:12
    semitone_ratios(i+1) = 2^(i/12);
end

% Display pattern information
printf('/* Arpeggiator Pattern Generator */\n');
printf('/* Sample Rate: %d Hz */\n', fs);
printf('/* BPM: %d */\n', bpm);
printf('/* Samples per step: %d */\n', samples_per_step);
printf('/* Pattern length: %d steps */\n\n', pattern_length);

% Generate C++ array for samples per step
printf('static constexpr uint32_t arp_samples_per_step = %d;\n\n', samples_per_step);

% Generate C++ array for pattern length
printf('static constexpr uint32_t arp_pattern_length = %d;\n\n', pattern_length);

% Generate C++ arrays for each pattern
patterns = {pattern_up, pattern_down, pattern_updown, pattern_random, pattern_octave};
pattern_names = {'up', 'down', 'updown', 'random', 'octave'};

for p = 1:length(patterns)
    pattern = patterns{p};
    name = pattern_names{p};
    
    printf('/* Pattern: %s */\n', name);
    printf('static constexpr std::array<uint8_t, %d> arp_pattern_%s\n{\n    ', pattern_length, name);
    
    for i = 1:pattern_length-1
        printf('%d, ', pattern(i));
    end
    printf('%d\n};\n\n', pattern(pattern_length));
end

% Generate C++ array for semitone pitch ratios
printf('/* Pitch shift ratios for semitones 0-12 (equal temperament) */\n');
printf('static constexpr std::array<float, 13> arp_semitone_ratios\n{\n    ');

for i = 1:12
    printf('%.16f, ', semitone_ratios(i));
end
printf('%.16f\n};\n\n', semitone_ratios(13));

% Visualize patterns
figure('Position', [100, 100, 1200, 800]);

for p = 1:length(patterns)
    subplot(3, 2, p);
    pattern = patterns{p};
    name = pattern_names{p};
    
    stem(1:pattern_length, pattern, 'filled', 'LineWidth', 2);
    grid on;
    title(['Arpeggiator Pattern: ' upper(name)]);
    xlabel('Step');
    ylabel('Semitones');
    ylim([-1, 13]);
    xlim([0, pattern_length+1]);
end

% Generate envelope shape for smooth transitions
envelope_samples = round(samples_per_step * 0.1); % 10% of step duration for attack/release
attack = linspace(0, 1, envelope_samples)';
release = linspace(1, 0, envelope_samples)';
sustain = ones(samples_per_step - 2*envelope_samples, 1);
envelope = [attack; sustain; release];

% Plot envelope
subplot(3, 2, 6);
plot(envelope, 'LineWidth', 2);
grid on;
title('Step Envelope (Attack-Sustain-Release)');
xlabel('Sample');
ylabel('Amplitude');
ylim([0, 1.1]);

% Generate C++ array for envelope
printf('/* Envelope for smooth step transitions */\n');
printf('static constexpr uint32_t arp_envelope_length = %d;\n', length(envelope));
printf('static constexpr std::array<float, %d> arp_envelope\n{\n', length(envelope));

cols = 6;
rows = ceil(length(envelope) / cols);
i = 1;
while rows > 0
    c = cols;
    if length(envelope) - i + 1 < c
        c = length(envelope) - i + 1;
    end
    rows = rows - 1;
    printf('    ');
    while c > 1
        printf('%.16f, ', envelope(i));
        c = c - 1;
        i = i + 1;
    end
    if i <= length(envelope)
        if i == length(envelope)
            printf('%.16f\n', envelope(i));
        else
            printf('%.16f,\n', envelope(i));
        end
        i = i + 1;
    end
end
printf('};\n\n');

printf('/* Usage Notes:\n');
printf(' * 1. Select a pattern (up, down, updown, random, octave)\n');
printf(' * 2. For each step, multiply input signal by semitone_ratios[pattern[step]]\n');
printf(' * 3. Apply envelope to smooth transitions between steps\n');
printf(' * 4. Advance to next step every arp_samples_per_step samples\n');
printf(' * 5. Loop back to step 0 after reaching arp_pattern_length\n');
printf(' */\n');