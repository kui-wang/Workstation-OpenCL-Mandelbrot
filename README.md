# Workstation-OpenCL-Mandelbrot
An OpenCL program implemented in C. The command-line program computes the Mandelbrot set with an escape time algorithm. It generates a bmp image visualizing the computation result, and a bin file containing the raw escape counts for all the numbers (pixels) computed. The program accepts two arguments, i.e. the resolution (e.g. 1024) and the assigned workload (an integer value from 1 to 4, where 1 means the bottom 25% of image and 4 means the full image). The program runs on a cloud computing infrastructure and accepts requests from clients (e.g. an Android phone) to compute a part of or a complete Mandelbrot set.