clear all
close all
clc

pkg load signal
pkg load control

output_precision(8)

% Script creates FIR filter based on impulse response of speaker cabinet
% FIR filter is used to emulate sound of speaker cabinet

% Load impulse response
wav_ir = '1960-G12M25-Starter/SM57/1960-G12M25-SM57-Cap45-0_5in.wav';
ir = wavread(wav_ir);
%ir = ir(1:4096);

% Plot frequency response from impulse response
fs = 48000;
n_fft = 4096;
[h,f] = freqz(ir,1,n_fft,fs);
[h,f] = freqz(ir,max(abs(h)),n_fft,fs); % scale to not exceed 0dB
figure(1);
semilogx(f,mag2db(abs(h))); grid on; hold on;
xlabel('Frequency (Hz)'); ylabel('Magnitude (dB)');

% Design FIR filter using least squares method
f = (f./(fs/2))';
h = abs(h');
fir_order = 512;
b = firls(fir_order,f,h);
[fir_h,fir_f] = freqz(b,1,n_fft,fs);

% Plot frequency response of FIR filter
semilogx(fir_f,mag2db(abs(fir_h)), 'r-');
xlabel('Frequency (Hz)'); ylabel('Magnitude (dB)');
legend('Original response', 'FIR response')

% Reverse order of coeffs (needed for CMSIS DSP)
coeffs = fliplr(b);

% Format coeffs to C++ std::array
vname = 'fir_coeffs';
vtype = 'float';
N = length(coeffs);
fmt=['std::array<%s, %d> %s{' repmat('%.8f,',1,numel(coeffs)-1) '%.8f}'];
c_code=sprintf(fmt,vtype,N,vname,coeffs)