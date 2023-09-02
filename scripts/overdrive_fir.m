clear all
close all
clc

pkg load signal;
output_precision(8);

% Design a lowpass FIR filter to attenuate freqs above fs/4 
fs = 48000;
fc = 11000;
fc_norm = fc/(fs/2);
b = fir1(96,fc_norm);
freqz(b,1,512,fs);

% Reverse order of coeffs (needed for CMSIS DSP)
coeffs = fliplr(b);

% Format coeffs to C++ std::array
vname = 'fir_coeffs';
vtype = 'float';
N = length(coeffs);
fmt=['std::array<%s, %d> %s{' repmat('%.8f,',1,numel(coeffs)-1) '%.8f}'];
c_code=sprintf(fmt,vtype,N,vname,coeffs)