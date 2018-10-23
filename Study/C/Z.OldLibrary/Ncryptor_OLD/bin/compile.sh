# ===========================================================
# [ PATH ]
BIN_DIR=`pwd`
PRJ_DIR=$BIN_DIR/..
SRC_DIR=$PRJ_DIR/src
RES_DIR=$PRJ_DIR/res
LIB_DIR=$SRC_DIR/ncrypt
# ===========================================================
# [ OUTPUT PATH ]
EXE_OUT_PATH=$BIN_DIR
EXE_NAME=ncrypt
# ===========================================================
# [ COMMON_CODE ]
C01=ncryptor_main.c
# [ LIB_CODE ]
EC01=$LIB_DIR/ncryptor.c
EC02=$LIB_DIR/ncrypt_seed.c
EC03=$LIB_DIR/KISA_SEED.c
# [ COMMON_LIBRARY ]
CL01=rbmath.c
# ===========================================================
# [ Compile and build ]
cd $SRC_DIR
gcc -o $EXE_OUT_PATH/$EXE_NAME \
$C01 $EC01 $EC02 $EC03 $CL01 \
-I $SRC_DIR -I $LIB_DIR
cd $BIN_DIR
# ===========================================================
chmod 755 $EXE_OUT_PATH/$EXE_NAME
echo "[$EXE_NAME] Compile done."
# ===========================================================