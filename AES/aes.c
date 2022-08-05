#include "aes.h"

struct AES* AESFactory(AES_t type)
{
	struct AES* aes = (struct AES*)malloc(sizeof *aes);
	aes->Nb = 4;
	switch (type)
	{
	case AES_128:
		aes->Nk = 4;
		aes->Nr = 10;
		break;
	case AES_192:
		aes->Nk = 6;
		aes->Nr = 12;
		break;
	case AES_256:
		aes->Nk = 8;
		aes->Nr = 14;
		break;
	default:
		break;
	}
	aes->blockBytesLen = aes->Nb * 4;
	return aes;
}

#define xtime(v) ((v << 1) ^ (((v >> 7) & 1) * 0x1b))

/**
 * Convert four byte s to one word.
 */
word Word(byte* k1, byte* k2, byte* k3, byte* k4)
{
	word result = 0;
	byte* asArray = (byte*)&result;
	asArray[0] = *k1;
	asArray[1] = *k2;
	asArray[2] = *k3;
	asArray[3] = *k4;
	return result;
}

/**
 *  Rotl
 */
void RotWord(byte* rw)
{
	unsigned char c = rw[0];
	rw[0] = rw[1];
	rw[1] = rw[2];
	rw[2] = rw[3];
	rw[3] = c;
}

/**
 *  S-box transformation for each byte in input word
 */
void SubWord(byte* a) {
	int i;
	for (i = 0; i < 4; i++) {
		a[i] = sbox[a[i] / 16][a[i] % 16];
	}
}

/*
*  S-box transformation for each byte inside the state matrix
*/
void SubBytes(struct AES* thisptr, byte** state) {
	int i, j;
	unsigned char t;
	for (i = 0; i < 4; i++) {
		for (j = 0; j < thisptr->Nb; j++) {
			t = state[i][j];
			state[i][j] = sbox[t / 16][t % 16];
		}
	}
}

void InvSubBytes(struct AES* thisptr, byte** state) {
	int i, j;
	unsigned char t;
	for (i = 0; i < 4; i++) {
		for (j = 0; j < thisptr->Nb; j++) {
			t = state[i][j];
			state[i][j] = inv_sbox[t / 16][t % 16];
		}
	}
}

/*
*  Generates round constants
*/
void Rcon(byte* pRcon, int n) {
	int i;
	byte c = 1;
	for (i = 0; i < n - 1; i++) {
		c = xtime(c);
	}

	pRcon[0] = c;
	pRcon[1] = pRcon[2] = pRcon[3] = 0;
}

/*
*  Given a state struct ptr
*/
void ShiftRow(struct AES* thisptr , byte** state, int i, int n) {
	byte* tmp = (byte*)malloc(thisptr->Nb);
	ZeroMemory(tmp, thisptr->Nb);
	for (int j = 0; j < thisptr->Nb; j++) {
		tmp[j] = state[i][(j + n) % thisptr->Nb];
	}
	memcpy(state[i], tmp, thisptr->Nb * sizeof(byte));

	free(tmp);
}

/*
*  Shifts all the rows according to the AES standard
*/
void ShiftRows(struct AES* thisptr, byte** state) {
	ShiftRow(thisptr, state, 1, 1);
	ShiftRow(thisptr, state, 2, 2);
	ShiftRow(thisptr, state, 3, 3);
}

void InvShiftRows(struct AES* thisptr, byte** state) {
	ShiftRow(thisptr, state, 1, thisptr->Nb - 1);
	ShiftRow(thisptr, state, 2, thisptr->Nb - 2);
	ShiftRow(thisptr, state, 3, thisptr->Nb - 3);
}

/*
*  Last step in the encryption algorithm, before adding the last round's key
*/
void MixColumns(byte** state) {
	unsigned char temp_state[4][4];

	for (size_t i = 0; i < 4; ++i) {
		memset(temp_state[i], 0, 4);
	}

	for (size_t i = 0; i < 4; ++i) {
		for (size_t k = 0; k < 4; ++k) {
			for (size_t j = 0; j < 4; ++j) {
				if (CMDS[i][k] == 1)
					temp_state[i][j] ^= state[k][j];
				else
					temp_state[i][j] ^= GF_MUL_TABLE[CMDS[i][k]][state[k][j]];
			}
		}
	}

	for (size_t i = 0; i < 4; ++i) {
		memcpy(state[i], temp_state[i], 4);
	}
}

void InvMixColumns(byte** state) {
	unsigned char temp_state[4][4];

	for (size_t i = 0; i < 4; ++i) {
		memset(temp_state[i], 0, 4);
	}

	for (size_t i = 0; i < 4; ++i) {
		for (size_t k = 0; k < 4; ++k) {
			for (size_t j = 0; j < 4; ++j) {
				temp_state[i][j] ^= GF_MUL_TABLE[INV_CMDS[i][k]][state[k][j]];
			}
		}
	}

	for (size_t i = 0; i < 4; ++i) {
		memcpy(state[i], temp_state[i], 4);
	}
}

/*
*  Last step of each iteration
*/
void AddRoundKey(struct AES* thisptr, byte** state, const byte* key) {
	int i, j;
	for (i = 0; i < 4; i++) {
		for (j = 0; j < thisptr->Nb; j++) {
			state[i][j] = state[i][j] ^ key[i + 4 * j];
		}
	}
}

/*
*  Xor an arbitrary number of bytes from in1 with in2, and store the result in out
*/
void XorBlocks(const unsigned char* in1, const unsigned char* in2,
	unsigned char* out, unsigned int len) {
	for (unsigned int i = 0; i < len; i++) {
		out[i] = in1[i] ^ in2[i];
	}
}

/*
*  Xors each byte of in1 with in2 and puts them inside out
*/
void XorWords(byte* in1, byte* in2, byte* out) {
	XorBlocks(in1, in2, out, 4);
}

/*
*  Extends key to be served for all rounds
*/
word* KeyExpansion(struct AES* thisptr, const byte* key, byte* w)
{
	byte* temp = (byte*)malloc(4);
	byte* rcon = (byte*)malloc(4);

	int i = 0;
	while (i < 4 * thisptr->Nk) {
		w[i] = key[i];
		i++;
	}

	i = 4 * thisptr->Nk;
	while (i < 4 * thisptr->Nb * (thisptr->Nr + 1)) {
		temp[0] = w[i - 4 + 0];
		temp[1] = w[i - 4 + 1];
		temp[2] = w[i - 4 + 2];
		temp[3] = w[i - 4 + 3];

		if (i / 4 % thisptr->Nk == 0) {
			RotWord(temp);
			SubWord(temp);
			Rcon(rcon, i / (thisptr->Nk * 4));
			XorWords(temp, rcon, temp);
		}
		else if (thisptr->Nk > 6 && i / 4 % thisptr->Nk == 4) {
			SubWord(temp);
		}

		w[i + 0] = w[i - 4 * thisptr->Nk] ^ temp[0];
		w[i + 1] = w[i + 1 - 4 * thisptr->Nk] ^ temp[1];
		w[i + 2] = w[i + 2 - 4 * thisptr->Nk] ^ temp[2];
		w[i + 3] = w[i + 3 - 4 * thisptr->Nk] ^ temp[3];
		i += 4;
	}

	free(rcon);
	free(temp);
}

void EncryptBlock(struct AES* thisptr, const byte* in, byte* out, const byte* roundKeys) {
	byte** state = (byte**) malloc(sizeof *state * 4);
	state[0] = (byte*)malloc(4 * thisptr->Nb);
	int i, j, round;
	for (i = 0; i < 4; i++) {
		state[i] = state[0] + thisptr->Nb * i;
	}

	for (i = 0; i < 4; i++) {
		for (j = 0; j < thisptr->Nb; j++) {
			state[i][j] = in[i + 4 * j];
		}
	}

	AddRoundKey(thisptr, state, roundKeys);

	for (round = 1; round <= thisptr->Nr - 1; round++) {
		SubBytes(thisptr, state);
		ShiftRows(thisptr, state);
		MixColumns(state);
		AddRoundKey(thisptr, state, roundKeys + round * 4 * thisptr->Nb);
	}

	SubBytes(thisptr, state);
	ShiftRows(thisptr, state);
	AddRoundKey(thisptr, state, roundKeys + thisptr->Nr * 4 * thisptr->Nb);

	for (i = 0; i < 4; i++) {
		for (j = 0; j < thisptr->Nb; j++) {
			out[i + 4 * j] = state[i][j];
		}
	}

	free(*state);
	free(state);
}

void DecryptBlock(struct AES* thisptr, const byte in[], byte out[],
	byte* roundKeys) {
	byte** state = (byte*)malloc(sizeof * state * 4);
	state[0] = (byte*)malloc(4 * thisptr->Nb);
	int i, j, round;
	for (i = 0; i < 4; i++) {
		state[i] = state[0] + thisptr->Nb * i;
	}

	for (i = 0; i < 4; i++) {
		for (j = 0; j < thisptr->Nb; j++) {
			state[i][j] = in[i + 4 * j];
		}
	}

	AddRoundKey(thisptr, state, roundKeys + thisptr->Nr * 4 * thisptr->Nb);

	for (round = thisptr->Nr - 1; round >= 1; round--) {
		InvSubBytes(thisptr, state);
		InvShiftRows(thisptr, state);
		AddRoundKey(thisptr, state, roundKeys + round * 4 * thisptr->Nb);
		InvMixColumns(state);
	}

	InvSubBytes(thisptr, state);
	InvShiftRows(thisptr, state);
	AddRoundKey(thisptr, state, roundKeys);

	for (i = 0; i < 4; i++) {
		for (j = 0; j < thisptr->Nb; j++) {
			out[i + 4 * j] = state[i][j];
		}
	}

	free(*state);
	free(state);
}

byte* EncryptECB(struct AES* thisptr, const byte in[], unsigned int inLen,
	const byte key[]) {
	if (inLen % 16) {
		return 0; //If length is invalid return a nullptr
	}
	byte* out = (byte*)malloc(inLen);
	byte* roundKeys = (byte*)malloc(4 * thisptr->Nb * (thisptr->Nr + 1));
	KeyExpansion(thisptr, key, roundKeys);
	for (unsigned int i = 0; i < inLen; i += thisptr->blockBytesLen) {
		EncryptBlock(thisptr, in + i, out + i, roundKeys);
	}

	free(roundKeys);

	return out;
}

byte* DecryptECB(struct AES* thisptr, const byte in[], unsigned int inLen, const byte key[])
{
	if (inLen % 16) {
		return 0; //If length is invalid return a nullptr
	}
	byte* out = (byte*)malloc(inLen);
	byte* roundKeys = (byte*)malloc(4 * thisptr->Nb * (thisptr->Nr + 1));
	KeyExpansion(thisptr, key, roundKeys);
	for (unsigned int i = 0; i < inLen; i += thisptr->blockBytesLen) {
		DecryptBlock(thisptr, in + i, out + i, roundKeys);
	}

	free(roundKeys);

	return out;
}

byte* EncryptCBC(struct AES* thisptr, const byte in[], unsigned int inLen, const byte key[], const byte* iv)
{
	if (inLen % 16) {
		return 0; //If length is invalid return a nullptr
	}
	byte* out = (byte*)malloc(inLen);
	byte* block = (byte*)malloc(thisptr->blockBytesLen);
	byte* roundKeys = (byte*)malloc(4 * thisptr->Nb * (thisptr->Nr + 1));
	KeyExpansion(thisptr, key, roundKeys);
	memcpy(block, iv, thisptr->blockBytesLen);
	for (unsigned int i = 0; i < inLen; i += thisptr->blockBytesLen) {
		XorBlocks(block, in + i, block, thisptr->blockBytesLen);
		EncryptBlock(thisptr, block, out + i, roundKeys);
		memcpy(block, out + i, thisptr->blockBytesLen);
	}

	free(block);
	free(roundKeys);

	return out;
}

byte* DecryptCBC(struct AES* thisptr, const byte in[], unsigned int inLen, const byte key[], const byte* iv)
{
	if (inLen % 16) {
		return 0; //If length is invalid return a nullptr
	}
	byte* out = (byte*)malloc(inLen);
	byte* block = (byte*)malloc(thisptr->blockBytesLen);
	byte* roundKeys = (byte*)malloc(4 * thisptr->Nb * (thisptr->Nr + 1));
	KeyExpansion(thisptr, key, roundKeys);
	memcpy(block, iv, thisptr->blockBytesLen);
	for (unsigned int i = 0; i < inLen; i += thisptr->blockBytesLen) {
		DecryptBlock(thisptr, in + i, out + i, roundKeys);
		XorBlocks(block, out + i, out + i, thisptr->blockBytesLen);
		memcpy(block, in + i, thisptr->blockBytesLen);
	}

	free(block);
	free(roundKeys);

	return out;
}

byte* EncryptCFB(struct AES* thisptr, const byte in[], unsigned int inLen, const byte key[], const byte* iv)
{
	if (inLen % 16) {
		return 0; //If length is invalid return a nullptr
	}
	byte* out = (byte*)malloc(inLen);
	byte* block = (byte*)malloc(thisptr->blockBytesLen);
	byte* encryptedBlock = (byte*)malloc(thisptr->blockBytesLen);
	byte* roundKeys = (byte*)malloc(4 * thisptr->Nb * (thisptr->Nr + 1));
	KeyExpansion(thisptr, key, roundKeys);
	memcpy(block, iv, thisptr->blockBytesLen);
	for (unsigned int i = 0; i < inLen; i += thisptr->blockBytesLen) {
		EncryptBlock(thisptr, block, encryptedBlock, roundKeys);
		XorBlocks(in + i, encryptedBlock, out + i, thisptr->blockBytesLen);
		memcpy(block, out + i, thisptr->blockBytesLen);
	}

	free(block);
	free(encryptedBlock);
	free(roundKeys);

	return out;
}

byte* DecryptCFB(struct AES* thisptr, const byte in[], unsigned int inLen, const byte key[], const byte* iv)
{
	if (inLen % 16) {
		return 0; //If length is invalid return a nullptr
	}
	byte* out = (byte*)malloc(inLen);
	byte* block = (byte*)malloc(thisptr->blockBytesLen);
	byte* encryptedBlock = (byte*)malloc(thisptr->blockBytesLen);
	byte* roundKeys = (byte*)malloc(4 * thisptr->Nb * (thisptr->Nr + 1));
	KeyExpansion(thisptr, key, roundKeys);
	memcpy(block, iv, thisptr->blockBytesLen);
	for (unsigned int i = 0; i < inLen; i += thisptr->blockBytesLen) {
		EncryptBlock(thisptr, block, encryptedBlock, roundKeys);
		XorBlocks(in + i, encryptedBlock, out + i, thisptr->blockBytesLen);
		memcpy(block, in + i, thisptr->blockBytesLen);
	}

	free(block);
	free(encryptedBlock);
	free(roundKeys);

	return out;
}
