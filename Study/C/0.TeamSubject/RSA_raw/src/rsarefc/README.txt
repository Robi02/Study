* rsarefc는 rsaref2.0이 기존에 사용하던 명명규칙(소스일부 수정시 파일명변경: des.c -> desc.c)에 따라 rsaref를 rsarefc로 명명함
* rsaref2.0 에서 개인키 암/복호화, 공개키 암/복호화 기능을 위해 필요한 부분을 제외한 나머지 소스는 제거함(des.h,desc.c,md2.h,md2c.c,md5.h,md5c.c,prime.h,prime.c,r_encode.h,r_encode.c,r_enhanc,r_keygen.c,r_random.h,r_random.c)
* rsa.c의 공개키 암호화(RSAPublicEncrypt) 함수의 마지막 파라미터는 난수구조체를 받아 내부적으로 난수를 연산하는 방식에서 외부에서 modulus크기 만큼 난수를 입력받도록 일부 프로그램을 수정함
* rsaref.h에서 MAX_RSA_MODULUS_BITS 1024 -> 2048로 rsa2048지원 추가
* rsaref.h에서 "Random structure."(55~63, 89~96) , "Cryptographic enhancements."(97~119) "Key-pair generation."(120~125) 삭제
