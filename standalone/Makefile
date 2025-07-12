.PHONY: all clean synchro

EXT = CryptoLW/Speck.o Crypto/BLAKE2s.o Crypto/ChaCha.o Crypto/Poly1305.o Crypto/Cipher.o Crypto/Hash.o Crypto/Crypto.o Crypto/BlockCipher.o
OBJS = importexport.o crypto.o utils.o String.o $(EXT)
CFLAGS = -I. -ICrypto -ICryptoLW/ -ggdb

all: synchro decode encode test

synchro:
	./sync

decode: $(OBJS) decode.o
	$(CXX) -o $@ $^

encode: $(OBJS) encode.o
	$(CXX) -o $@ $^

test: $(OBJS) main.o
	$(CXX) -o $@ $^

%.o: %.cpp
	$(CXX) $(CFLAGS) -o $@ -c $^

clean:
	rm decode $(OBJS)
