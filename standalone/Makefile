.PHONY: all clean synchro

EXT = String.o Crypto/BLAKE2s.o Crypto/ChaCha.o Crypto/Poly1305.o Crypto/Cipher.o Crypto/Hash.o Crypto/Crypto.o Crypto/BlockCipher.o
OBJS = streamadapter.o importexport.o crypto.o utils.o $(EXT)
CFLAGS = -I. -ICrypto -ggdb -W -Wall -fPIC -DENCRYPTED_PASS=0
BINS = generate encoderaw decoderaw


all: synchro $(BINS) libkeypass.so

synchro:
	./sync

generate: $(OBJS) generate.o
	$(CXX) -o $@ $^

encoderaw: $(OBJS) encoderaw.o
	$(CXX) -o $@ $^

decoderaw: $(OBJS) decoderaw.o restore.o
	$(CXX) -o $@ $^

libkeypass.so: Crypto/BLAKE2s.o Crypto/BlockCipher.o Crypto/ChaCha.o Crypto/Cipher.o Crypto/Crypto.o Crypto/Hash.o String.o _keypass.o crypto.o importexport.o utils.o
	$(CXX) -shared -o $@ $^

%.o: %.cpp
	$(CXX) $(CFLAGS) -o $@ -c $^

clean:
	rm -fr $(BINS) $(OBJS) *.o *.so build
