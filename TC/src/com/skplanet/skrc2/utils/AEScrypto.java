/*
	* The MIT License (MIT)
	* Copyright (c) 2015 SK PLANET. All Rights Reserved.
	*
	* Permission is hereby granted, free of charge, to any person obtaining a copy
	* of this software and associated documentation files (the "Software"), to deal
	* in the Software without restriction, including without limitation the rights
	* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	* copies of the Software, and to permit persons to whom the Software is
	* furnished to do so, subject to the following conditions:
	*
	* The above copyright notice and this permission notice shall be included in
	* all copies or substantial portions of the Software.
	*
	* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	* THE SOFTWARE.
	*/


package com.skplanet.skrc2.utils;

import java.security.NoSuchAlgorithmException;

import javax.crypto.Cipher;
import javax.crypto.NoSuchPaddingException;
import javax.crypto.spec.SecretKeySpec;

public class AEScrypto {

	static final private String DEFAULT_KEY = "57c3038cad712da9"; // season2

	static final private String CRYPT_ALGORITHM = "AES";
	
	static final private String CRYPT_TRANSFORM = "AES/ECB/PKCS5Padding";
	
	private Cipher cipher;
	private String cryptKey = null;
	private String encrypt_str = null;
	private String decrypt_str = null;
	
	public AEScrypto(){
		try{
			cipher = Cipher.getInstance(CRYPT_TRANSFORM);
		}catch(NoSuchAlgorithmException nsae){
			throw new RuntimeException(nsae);
		}catch(NoSuchPaddingException nspe){
			throw new RuntimeException(nspe);
		}
	}
	
	public void useDefaultKey() {
		this.cryptKey = DEFAULT_KEY;
	}
	
	public void setCryptKey(String cryptKey) {
		this.cryptKey = cryptKey;
	}
	
	public void setEncrypt_str(String encrypt_str) {
		this.encrypt_str = encrypt_str;
	}
	
	public void setDecrypt_str(String decrypt_str) {
		this.decrypt_str = decrypt_str;
	}
	
	public String runEncryption() throws Exception{
		byte[] raw = cryptKey.getBytes();
		SecretKeySpec skeySpec = new SecretKeySpec(raw, CRYPT_ALGORITHM);
		
		byte[] encrypted = null;
		synchronized(cipher){
			cipher.init(Cipher.ENCRYPT_MODE, skeySpec);
			encrypted = cipher.doFinal(encrypt_str.getBytes());
		}
		return toHexString(encrypted, 0, encrypted.length);
	}
	
	public String runDecryption() throws Exception{
		byte[] raw = cryptKey.getBytes();
		SecretKeySpec skeySpec = new SecretKeySpec(raw, CRYPT_ALGORITHM);
		byte[] encrypted = fromHexaStr(decrypt_str);
		byte[] decrypted = null;
		synchronized(cipher){
			cipher.init(Cipher.DECRYPT_MODE, skeySpec);
			decrypted = cipher.doFinal(encrypted);
		}
		return new String(decrypted);
	}
	
	public static String toHexString(byte[] b, int off, int len) {
		StringBuffer sb = new StringBuffer();
		for (int i = 0; i < len; i++) {
			char c1, c2;
			c1 = (char)((b[off+i]>>>4)&0xf);
			c2 = (char)(b[off+i]&0xf);
			c1 = (char)((c1 > 9) ? 'a'+(c1-10) : '0'+c1);
			c2 = (char)((c2 > 9) ? 'a'+(c2-10) : '0'+c2);
			sb.append(c1);
			sb.append(c2);
		}
		return sb.toString();
	}
	
	private static byte[] fromHexaStr(String hexaStr){
		int len = hexaStr.length();
		byte[] buffer = new byte[((len + 1) / 2)];
		int i = 0, j = 0;
		if ((len % 2) != 0)
			buffer[j++] = (byte)fromDigitChar(hexaStr.charAt(i++));
		while (i < len) {
			buffer[j++] = (byte)((fromDigitChar(hexaStr.charAt(i++)) << 4)
			| fromDigitChar(hexaStr.charAt(i++)));
		}
		return buffer;
	}
	
	private static int fromDigitChar(char ch) {
		if (ch >= '0' && ch <= '9')
			return ch - '0';
		if (ch >= 'A' && ch <= 'F')
			return ch - 'A' + 10;
		if (ch >= 'a' && ch <= 'f')
			return ch - 'a' + 10;
		throw new IllegalArgumentException("Invalid hexa digit '" + ch + "'");
	}
	
	//암호화
	public static String enCrypto(String str){
		String result = "";
		try {
			AEScrypto cryptology = new AEScrypto();
			cryptology.useDefaultKey();
			cryptology.setEncrypt_str(str);
			result = cryptology.runEncryption();
		
		} catch (Exception e) {
			// TODO: handle exception
		}
		return result;
	}
	
	//복호화
	public static String deCrypto(String str){
		String result = "";
		try {
			AEScrypto cryptology = new AEScrypto();
			cryptology.useDefaultKey();
			cryptology.setDecrypt_str(str);
			result = cryptology.runDecryption();
		
		} catch (Exception e) {
			// TODO: handle exception
		}
		return result;
	}
	


}
