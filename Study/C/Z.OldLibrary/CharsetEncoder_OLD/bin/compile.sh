# ===========================================================
# [ PATH ]
BIN_DIR=`pwd`
PRJ_DIR=$BIN_DIR/..
SRC_DIR=$PRJ_DIR/src
RES_DIR=$PRJ_DIR/res
LIB_ENC_DIR=$SRC_DIR/encode
# ===========================================================
# [ OUTPUT PATH ]
EXE_OUT_PATH=$BIN_DIR
EXE_NAME=encode
# ===========================================================
# [ COMMON_CODE ]
C01=main.c
C02=encoder.c
# [ ENC_CODE ]
EC01=$LIB_ENC_DIR/enc_hexa.c
EC02=$LIB_ENC_DIR/enc_base64.c
# [ COMMON_LIBRARY ]
CL01=rbmath.c
# ===========================================================
# [ Compile and build ]
cd $SRC_DIR
gcc -o $EXE_OUT_PATH/$EXE_NAME \
$C01 $C02 $EC01 $EC02 $CL01 \
-I $SRC_DIR -I $LIB_ENC_DIR
cd $BIN_DIR
# ===========================================================
chmod 755 $EXE_OUT_PATH/$EXE_NAME
echo "[$EXE_NAME] Compile done."
# ===========================================================