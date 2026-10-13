# Classic Kaleidoscope Mandelbrot Set Visualizer
# Emits ASCII visualization using putchard()

extern putchard(x);

def printdensity(d)
  if d > 8 then
    putchard(32)  # ' '
  else if d > 4 then
    putchard(46)  # '.'
  else if d > 2 then
    putchard(43)  # '+'
  else
    putchard(42); # '*'

def mandelconverge(real imag iters creal cimag)
  if iters > 255 then
    1
  else if (real * real + imag * imag) > 4 then
    iters
  else
    mandelconverge(real * real - imag * imag + creal,
                   2 * real * imag + cimag,
                   iters + 1, creal, cimag);

def mandelhelp(xmin xmax xstep ymin ymax ystep)
  for y = ymin, y < ymax, ystep in (
    (for x = xmin, x < xmax, xstep in
       printdensity(mandelconverge(x, y, 0, x, y)))
    + putchard(10) # newline
  );

def mandel(realstart imagstart realstep imagstep)
  mandelhelp(realstart, realstart + 78 * realstep, realstep,
             imagstart, imagstart + 40 * imagstep, imagstep);
