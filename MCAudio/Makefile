CC=gcc
CPPC=g++
CFLAGS= -g
LDLIBS=-lole32 -lsetupapi -luuid -lksguid -lntdll -llibfftw3f-3


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

mcaudiospectrum-obj:
	$(CC) $(CFLAGS) -c MCAudioSpectrum.c -o MCAudioSpectrum.o

mcaudiocli-obj: mcaudiolib
	$(CC) $(CFLAGS) -c MCAudioCLI.c -o MCAudioCLI.o

# Creating mcaudio static library
mcaudiolib: mcaudioinit-obj mcaudiodiscover-obj mcaudiowavesave-obj mcaudiolisten-obj mcaudiospectrum-obj mcaudiocli-obj
	ar rcs MCAudio.lib MCAudioSpectrum.o MCAudioDiscover.o MCAudioListen.o MCAudioWaveSave.o MCAudioInit.o MCAudioCLI.o

clean:
	rm -rf *.exe *.lib *.o *.wav


