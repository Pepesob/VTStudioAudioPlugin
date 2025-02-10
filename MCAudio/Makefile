CC=gcc
CPPC=g++
CFLAGS=
LDLIBS=-lole32 -lsetupapi -luuid

mcaudiomain: mcaudiolib
	$(CC) $(CFLAGS) -c main.c -o main.o
	$(CC) $(CFLAGS) main.o -o main.exe -L. -lMCAudio $(LDLIBS)

mcaudioobj:
	$(CC) $(CFLAGS) -c MCAudioDiscover.c -o MCAudioDiscover.o
	$(CC) $(CFLAGS) -c MCAudioListen.c -o MCAudioListen.o
	$(CC) $(CFLAGS) -c MCAudioWaveSave.c -o MCAudioWaveSave.o
	$(CC) $(CFLAGS) -c MCAudioInit.c -o MCAudioInit.o

mcaudiolib: mcaudioobj
	ar rcs MCAudio.lib MCAudioDiscover.o MCAudioListen.o MCAudioWaveSave.o MCAudioInit.o

clean:
	rm -rf *.exe *.lib *.o *.wav


