c = load('taps.txt');
c = c(:);
K = (numel(c) - 1) / 2;

fprintf(1, '[%s]: K = %i\n', mfilename(), K);

xy = load('test.txt');
assert(size(xy, 2) == 2);
N = size(xy, 1);
x = xy(:, 1);
y = xy(:, 2);

xaug = [x; zeros(K, 1)];
yaug = filter(c, 1, xaug);
yref = yaug((K + 1):end);
assert(numel(yref) == N);

max_abs_err = max(abs(y - yref));
fprintf(1, '[%s]: err = %e\n', mfilename(), max_abs_err);
if max_abs_err < 1e-14
  fprintf(1, 'test OK!\n');
else 
  warning('test is NOT OK; error is too large');
end 
