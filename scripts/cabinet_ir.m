clear all
close all
clc

pkg load signal
pkg load control

output_precision(8)

% Script generates C++ array of samples from IR wave file

% Load impulse response
N = 2048;
ir_file = '1960-G12M25-Starter/SM57/1960-G12M25-SM57-Cap45-0_5in.wav';
[~,ir_name,~] = fileparts(ir_file);
ir = wavread(ir_file);
ir = ir(1:N);

% Scale
ir_pow = sum(ir.*ir)/length(ir);
ir_gain = -10.0 * log10(ir_pow / 1);
ir = ir/ir_gain;

% Format IR samples to C++ std::array
type = 'float';
filename = "impulse_response.c";
fid = fopen(filename, "w");
fprintf(fid, '/* Impulse response: %s */\n', ir_name);
ir_name = ['ir_' strrep(ir_name, '-', '_')];
fprintf(fid, 'std::array<%s, %d> %s\n{\n', type, N, ir_name);

i = 1;
rows = 6;
while N > 2
  R = rows;
  if N < rows R = N; end
  N = N - R;
  while R > 0
    fprintf(fid, '    %.8f,', ir(i));
    R = R - 1;
    i = i + 1;
  end
  fprintf(fid, '\n');
end

fprintf(fid, '    %.8f\n};\n', ir(i));
fclose(fid);