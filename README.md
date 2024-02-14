# stimanography

`stimanograph` is a simple implementation of a [steganographic](https://en.wikipedia.org/wiki/Steganography) algorithm. 

This program hides a message inside an image by replacing the last bit of each byte in the image with a bit from the message. 

It can also be used to decrypt a message that has been hidden within an image, assuming it was encrypted with the same method as described above.

**Note:**
- image size must be 1024x1024 pixels
- image format must be `.bmp` (bitmap)
- message length cannot exceed 64 KB
- encryption process will append noise (random bits) to the message in the case that the message itself is not long enough to fill the entire image

**Usage:**
1. Clone this repository and navigate to it:
```
git clone https://github.com/0x726179616E/stimanography && cd stimanography
```
2. Compile the source code into an executable file named `stima` (or another name of your choosing):
``` 
cc main.c -o stima
```
3. Run the program in encryption mode:
```
./stima -e -m "here is my not so secret message" -o encrypted_img.bmp img.bmp 

successfully encrypted message to: encrypted_img.bmp
```
4. Run the program in decryption mode:
```
./stima -d encrypted_img.bmp 

decrypted message: here is my not so secret message...
```