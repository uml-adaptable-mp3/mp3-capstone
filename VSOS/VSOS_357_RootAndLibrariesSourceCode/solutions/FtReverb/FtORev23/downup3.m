subplot(311);
fs1   = 96000;
fs1p2 = fs1/2;
qf1  = firls(120, [0 15000/fs1p2 16000/fs1p2 1],[1 1 0 0],[1 1000000]);
qi1  = fir2vsdsp(qf1, 16, 1, 3, "down3L", "down3R", "-rate", fs1, "-file", "downup3");

subplot(312);
fs2   = 32000;
fs2p2 = fs2/2;
qf2  = firls(126, [0 15500/(fs2p2*3) 16500/(fs2p2*3) 1],[1 1 0 0],[1 1000000]);
qi2  = fir2vsdsp(qf2, 16, 3, 1, "up3L", "up3R", "-rate", fs2, "-file", "downup3", "-append");

subplot(313)
qf3  = conv(qf1,qf2);
qi3  = fir2vsdsp(qf3, 16, 1, 1, "up2", "-rate", fs1, "-file", "/tmp/downup3");
