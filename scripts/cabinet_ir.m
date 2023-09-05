clear all
close all
clc

pkg load signal
pkg load control

output_precision(8)

% Script generates C++ array of samples from IR wave file

% Load impulse response
ir_file = '1960-G12M25-Starter/SM57/1960-G12M25-SM57-Cap45-0_5in.wav';
[~,ir_name,~] = fileparts(ir_file);
ir = wavread(ir_file);

% Cut to desired length
N = 2048;
ir = ir(1:N);

% Apply window after cut (0 - rectangular ... 1 - hann)
w = tukeywin(2*length(ir), 0);
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
fs = 48000;
Nfft = 4096;
[h,f] = freqz(ir,1,Nfft,fs);
[h_w,f_w] = freqz(ir_w,1,Nfft,fs);
figure;
semilogx(f,mag2db(abs(h))); grid on; hold on;
semilogx(f_w,mag2db(abs(h_w)), '--r'); 
title('Frequency response'); xlabel('Frequency (Hz)'); ylabel('Magnitude (dB)');
legend('original', 'windowed');

% Scale to not exceed 0 dB
ir = ir_w;
ir_pow = sum(ir.*ir)/length(ir);
ir_gain = -10.0 * log10(ir_pow / 1);
ir = ir/ir_gain;

% Format IR samples to C++ std::array
type = 'float';
filename = "impulse_response.c";
fid = fopen(filename, "w");
fprintf(fid, '/* Impulse response: %s */\n', ir_name);
ir_name = ['ir_' strrep(ir_name, '-', '_')];
fprintf(fid, 'constexpr std::array<%s, %d> %s\n{\n', type, N, ir_name);

i = 1;
cols = 6;
rows = ceil(N / cols);
while rows > 0
  c = cols;
  if N - i + 1 < c
    c = rows;
  end
  rows = rows - 1;
  while c > 0
    fprintf(fid, '    %.8f,', ir(i));
    c = c - 1;
    i = i + 1;
  end
  fprintf(fid, '\n');
end
fseek(fid, ftell(fid)-1);
fprintf(fid, '    %.8f\n};\n', ir(i));
fclose(fid);