.PHONY: all clean synchro

OBJS = Crypto/BLAKE2s.o Crypto/ChaCha.o Crypto/Poly1305.o Crypto/Cipher.o Crypto/Hash.o Crypto/Crypto.o crypto.o utils.o main.o
CFLAGS=-ICrypto

all: synchro decode

synchro:
	./sync

decode: $(OBJS)
	$(CXX) -o $@ $^

%.o: %.cpp
	$(CXX) -o $@ -c $^ $(CFLAGS)

clean:
	rm decode $(OBJS)
