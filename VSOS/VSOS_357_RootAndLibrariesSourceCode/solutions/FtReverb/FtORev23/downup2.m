subplot(211);
fs1   = 96000;
fs1p2 = fs1/2;
qf1  = firls(28, [0 16000/fs1p2 24000/fs1p2 1],[1 1 0 0],[1 10000000]);
qi1  = fir2vsdsp(qf1, 16, 1, 2, "down2L", "down2R", "-rate", fs1, "-file", "downup2");

subplot(212);
fs2   = 48000;
fs2p2 = fs2/2;
qf2  = firls(48, [0 18000/fs2 30000/fs2 1],[1 1 0 0]);
qi2  = fir2vsdsp(qf2, 16, 2, 1, "up2L", "up2R", "-rate", fs2, "-file", "downup2", "-append");

%qf2(25)=-0.5;
%qf3  = conv(qf1,qf2);
%qi3  = fir2vsdsp(qf3, 16, 1, 1, "up2", "-rate", fs1);
