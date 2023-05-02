c = load('taps.txt');
c = c(:);
K = (numel(c) - 1) / 2;

disp(K);

xy = load('chirp.txt');
assert(size(xy, 2) == 2);
N = size(xy, 1);
x = xy(:, 1);
y = xy(:, 2);

yref = filter(c, 1, x);
disp(N);
disp(size(yref));
