CC=gcc
CPPC=g++
CFLAGS=
LDLIBS=-lole32 -lsetupapi -luuid -lksguid -lntdll


# Compiling main executable
mcaudiomain: mcaudiolib
	$(CC) $(CFLAGS) -c main.c -o main.o
	$(CC) $(CFLAGS) main.o -o main.exe -L. -lMCAudio $(LDLIBS)


# Compiling to object files
mcaudioinit-obj:
	$(CC) $(CFLAGS) -c MCAudioInit.c -o MCAudioInit.o

mcaudiodiscover-obj:
	$(CC) $(CFLAGS) -c MCAudioDiscover.c -o MCAudioDiscover.o

mcaudiowavesave-obj:
	$(CC) $(CFLAGS) -c MCAudioWaveSave.c -o MCAudioWaveSave.o

mcaudiolisten-obj:
	$(CC) $(CFLAGS) -c MCAudioListen.c -o MCAudioListen.o

# Creating mcaudio static library
mcaudiolib: mcaudioinit-obj mcaudiodiscover-obj mcaudiowavesave-obj mcaudiolisten-obj
	ar rcs MCAudio.lib MCAudioDiscover.o MCAudioListen.o MCAudioWaveSave.o MCAudioInit.o 

ffttest: mcaudiolib
	$(CC) $(CFLAGS) MCAudioSpectrum.c -o MCAudioSpectrum.exe -L. -lMCAudio -llibfftw3f-3 $(LDLIBS)

clean:
	rm -rf *.exe *.lib *.o *.wav


