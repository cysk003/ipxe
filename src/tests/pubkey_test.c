/*
 * Copyright (C) 2024 Michael Brown <mbrown@fensystems.co.uk>.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 * You can also choose to distribute this program under the terms of
 * the Unmodified Binary Distribution Licence (as given in the file
 * COPYING.UBDL), provided that you have satisfied its requirements.
 */

FILE_LICENCE ( GPL2_OR_LATER_OR_UBDL );

/** @file
 *
 * Public key self-tests
 *
 */

/* Forcibly enable assertions */
#undef NDEBUG

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ipxe/crypto.h>
#include <ipxe/test.h>
#include "pubkey_test.h"

/**
 * Report public key encryption and decryption test result
 *
 * @v test		Public key encryption and decryption test
 * @v file		Test code file
 * @v line		Test code line
 */
void pubkey_okx ( struct pubkey_test *test, const char *file,
		  unsigned int line ) {
	struct pubkey_algorithm *pubkey = test->pubkey;
	uint8_t private_ctx[pubkey->ctxsize];
	uint8_t public_ctx[pubkey->ctxsize];
	size_t max_len;

	/* Initialize contexts */
	okx ( pubkey_init ( pubkey, private_ctx, &test->private ) == 0,
	      file, line );
	okx ( pubkey_init ( pubkey, public_ctx, &test->public ) == 0,
	      file, line );
	max_len = pubkey_max_len ( pubkey, private_ctx );

	/* Test decrypting with private key to obtain known plaintext */
	{
		uint8_t decrypted[max_len];
		int decrypted_len;

		decrypted_len = pubkey_decrypt ( pubkey, private_ctx,
						 test->ciphertext,
						 test->ciphertext_len,
						 decrypted );
		okx ( decrypted_len == ( ( int ) test->plaintext_len ),
		      file, line );
		okx ( memcmp ( decrypted, test->plaintext,
			       test->plaintext_len ) == 0, file, line );
	}

	/* Test encrypting with private key and decrypting with public key */
	{
		uint8_t encrypted[max_len];
		uint8_t decrypted[max_len];
		int encrypted_len;
		int decrypted_len;

		encrypted_len = pubkey_encrypt ( pubkey, private_ctx,
						 test->plaintext,
						 test->plaintext_len,
						 encrypted );
		okx ( encrypted_len >= 0, file, line );
		decrypted_len = pubkey_decrypt ( pubkey, public_ctx,
						 encrypted, encrypted_len,
						 decrypted );
		okx ( decrypted_len == ( ( int ) test->plaintext_len ),
		      file, line );
		okx ( memcmp ( decrypted, test->plaintext,
			       test->plaintext_len ) == 0, file, line );
	}

	/* Test encrypting with public key and decrypting with private key */
	{
		uint8_t encrypted[max_len];
		uint8_t decrypted[max_len];
		int encrypted_len;
		int decrypted_len;

		encrypted_len = pubkey_encrypt ( pubkey, public_ctx,
						 test->plaintext,
						 test->plaintext_len,
						 encrypted );
		okx ( encrypted_len >= 0, file, line );
		decrypted_len = pubkey_decrypt ( pubkey, private_ctx,
						 encrypted, encrypted_len,
						 decrypted );
		okx ( decrypted_len == ( ( int ) test->plaintext_len ),
		      file, line );
		okx ( memcmp ( decrypted, test->plaintext,
			       test->plaintext_len ) == 0, file, line );
	}

	/* Free contexts */
	pubkey_final ( pubkey, public_ctx );
	pubkey_final ( pubkey, private_ctx );
}

/**
 * Report public key signature test result
 *
 * @v test		Public key signature test
 * @v file		Test code file
 * @v line		Test code line
 */
void pubkey_sign_okx ( struct pubkey_sign_test *test, const char *file,
		       unsigned int line ) {
	struct pubkey_algorithm *pubkey = test->pubkey;
	struct digest_algorithm *digest = test->digest;
	uint8_t private_ctx[pubkey->ctxsize];
	uint8_t public_ctx[pubkey->ctxsize];
	uint8_t digestctx[digest->ctxsize ];
	uint8_t digestout[digest->digestsize];
	size_t max_len;

	/* Initialize contexts */
	okx ( pubkey_init ( pubkey, private_ctx, &test->private ) == 0,
	      file, line );
	okx ( pubkey_init ( pubkey, public_ctx, &test->public ) == 0,
	      file, line );
	max_len = pubkey_max_len ( pubkey, private_ctx );

	/* Construct digest over plaintext */
	digest_init ( digest, digestctx );
	digest_update ( digest, digestctx, test->plaintext,
			test->plaintext_len );
	digest_final ( digest, digestctx, digestout );

	/* Test signing using private key */
	{
		uint8_t signature[max_len];
		int signature_len;

		signature_len = pubkey_sign ( pubkey, private_ctx, digest,
					      digestout, signature );
		okx ( signature_len == ( ( int ) test->signature_len ),
		      file, line );
		okx ( memcmp ( signature, test->signature,
			       test->signature_len ) == 0, file, line );
	}

	/* Test verification using public key */
	okx ( pubkey_verify ( pubkey, public_ctx, digest, digestout,
			      test->signature, test->signature_len ) == 0,
	      file, line );

	/* Test verification failure of modified signature */
	{
		uint8_t bad[test->signature_len];

		memcpy ( bad, test->signature, test->signature_len );
		bad[ test->signature_len / 2 ] ^= 0x40;
		okx ( pubkey_verify ( pubkey, public_ctx, digest, digestout,
				      bad, sizeof ( bad ) ) != 0, file, line );
	}

	/* Free contexts */
	pubkey_final ( pubkey, public_ctx );
	pubkey_final ( pubkey, private_ctx );
}