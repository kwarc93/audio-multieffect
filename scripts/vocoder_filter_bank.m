clear all
close all
clc

pkg load signal
pkg load control
output_precision(16)

% Script generates coefficients for SOS IIR bandpass filters

fs = 48000;
H = 2048;

% Bark scale center frequencies & bandwidths
bands_cf = [50+150 250+350 450+570 700+840 1000+1170 1370+1600 1850+2150 2500+2900 3400+4000 4800+5800 7000+8500 10500+13500]/2;
bands_bw = [80+100 100+100 110+120 140+150 160+190   210+240   280+320   380+450   550+700   900+1100  1300+1800 2500+3500]/3;
N = 12; % number of bands
fn = 2; % bandpass filter order = fn * 2
rip = 3; % passband ripple in dB
hsum = zeros(H,1); % Sum of filters frequency responses
tic
figure;
for n=1:N
  f0 = (bands_cf(n)-bands_bw(n))*2/fs;
  f1 = (bands_cf(n)+bands_bw(n))*2/fs;
  [z, p, k] = cheby1(fn, rip, [f0 f1]); % Chebyshev-type 1 filter design
  [b, a] = zp2tf(z, p, k);
  freqz(b,a,H,fs);
  [h, w] = freqz(b,a, H,fs);
  hsum = hsum + h;
  subplot(2, 1, 1); grid on; hold on;
  if n == N
    % Plot filters frequency responses
    plot(w, mag2db(abs(hsum)), 'k','linewidth', 2);
  endif
  subplot(2, 1, 2); grid on; hold on;
  
  % Generate c++ arrays of coeffs
  sos = zp2sos(z, p, k);
  fsos = [[sos(1, 1:3) sos(1, 5:6)]; [sos(2, 1:3) sos(2, 5:6)]];
  fsos(1, 4:5) = -fsos(1, 4:5);
  fsos(2, 4:5) = -fsos(2, 4:5); 
  printf('/* Band %d: %.1f Hz - %.1f Hz */\n', n, f0*fs/2, f1*fs/2);
  printf('static constexpr std::array<float, 10> bandpass_%d_coeffs\n{\n    ', n);
  printf('%.16f,', fsos(1, :)); printf('\n    ');
  printf('%.16f,', fsos(2, 1:4)); printf('%.16f', fsos(2, 5)); printf('\n};');
  printf('\n');
end
