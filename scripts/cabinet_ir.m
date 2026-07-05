clear all
close all
clc

pkg load signal
pkg load control

output_precision(16)

% Script generates C++ array of samples from IR wave file
fs = 48000;
len = 2048;

% Load impulse response

##ir_file = '..\IR\RedWirez IR\mixIR3\Library\IRs\Redwirez\BIGBoxX\48kHz-24bit\1960-G12M25-Starter\SM57\1960-G12M25-SM57-Cap45-0_5in.wav';
##ir_file = '..\IR\Guitar Impulses\Fearcomplexmusic Impulses\orange2x12.wav';
##ir_file = '..\IR\Guitar Impulses\Catharsis Fredman Impulses\1on-pres8.wav'
ir_file = '..\IR\Tone3000\V30 LR 4FB 4x12 SM57 1.00in 0.0in VP28.wav';
[~,ir_name,~] = fileparts(ir_file);
[ir_raw, ir_fs] = audioread(ir_file);

% Resample
if ir_fs != fs
ir = resample(ir_raw, fs, ir_fs);
else
ir = ir_raw;
end

% Pad zeros or cut
if length(ir) < len
ir = [ir; zeros(len - length(ir), 1)];
else
ir = ir(1:len);
end

% Apply window after cut (0 - rectangular ... 1 - hann)
w = tukeywin(2*length(ir), 0.01);
w = w(length(w)/2+1:end);
ir_w = ir.*w;

% Plot impulse response
figure;
plot(ir); grid on; hold on;
plot(ir_w, '--r');
plot(w, 'k');
title('Impulse response'); xlabel('Samples'); ylabel('Amplitude');
legend('original', 'windowed', 'window');

% Plot frequency response
Nfft = 4096;
[h,f] = freqz(ir,1,Nfft,fs);
[h_w,f_w] = freqz(ir_w,1,Nfft,fs);
figure;
semilogx(f,mag2db(abs(h))); grid on; hold on;
semilogx(f_w,mag2db(abs(h_w)), '--r');
title('Frequency response'); xlabel('Frequency (Hz)'); ylabel('Magnitude (dB)');
legend('original', 'windowed');

% Calculate average gain around 700 - 1500Hz
f_low = 700; f_high = 1500;
idx = find(f_w >= f_low & f_w <= f_high);
avg_mag = mean(abs(h_w(idx)));
avg_db = 20*log10(avg_mag);
% Scale down to -6dB around that freq. range
gain = 10^((avg_db + 6) / 20);
ir = ir_w/gain;

% Optionally plot frequency response with gain normalization
[h_w_s,f_w_s] = freqz(ir,1,Nfft,fs);
hold on;
semilogx(f_w_s,mag2db(abs(h_w_s)), 'k');
legend('original', 'windowed', 'scaled');

% Format IR samples to C++ std::array
type = 'float';
filename = "impulse_response.c";
fid = fopen(filename, "w");
fprintf(fid, '/* Impulse response: %s */\n', ir_name);
ir_name = ['ir_' strrep(ir_name, '-', '_')];
fprintf(fid, 'static constexpr std::array<%s, %d> %s\n{\n', type, len, ir_name);

i = 1;
cols = 3;
rows = ceil(len / cols);
while rows > 0
  c = cols;
  if len - i + 1 < c
    c = rows;
  end
  rows = rows - 1;
  while c > 0
    fprintf(fid, '    %.16f,', ir(i));
    c = c - 1;
    i = i + 1;
  end
  fprintf(fid, '\n');
end
fseek(fid, ftell(fid)-1);
fprintf(fid, '    %.16f\n};\n', ir(i));
fclose(fid);