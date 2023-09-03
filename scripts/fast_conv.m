% Performs 'overlap-save' fast convolution of signal x with impulse response h
% Parameters:
% x - input signal
% h - impulse response
% B - block size of input signal
% K - size of FFT
% Return:
% ret - result of convolution
function [ret] = fast_conv (x, h, B, K = 0)

M = length(x);
N = length(h);

if K == 0
% Next power of 2
K = max(B, 2^(floor(log2(N - 1)) + 1));
end

% Calculate the number of input blocks
num_input_blocks = ceil(M / B) + ceil(K / B) - 1;

% Pad x to an integer multiple of B
xp = [x; zeros(mod(M,num_input_blocks * B), 1)];

output_size = num_input_blocks * B + N - 1;
ret = zeros(output_size, 1);

% Input buffer
xw = zeros(K, 1);

% Pre compute FFT of h
H = fft([h; zeros(mod(K,length(h)), 1)]);
    
% Convolve all blocks
for n = 0:num_input_blocks
    % Extract the n-th input block
    xb = xp(n * B + 1:n * B + B);

    % Sliding window of the input
    xw = circshift(xw, -B);
    xw(end - B + 1:end) = xb;

    % Calculate the fast Fourier transforms of the time-domain signal x
    X = fft([xw; zeros(mod(K,length(xw)), 1)]);

    % Perform circular convolution in the frequency domain
    Y = X.*H;

    % Go back to time domain
    u = real(ifft(Y));

    % Save the valid output samples
    y(n * B + 1:n * B + B) = u(end - B + 1:end);
end
ret = y(1:M + N - 1);
endfunction
