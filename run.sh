OUTPUT_EXE_FILE="attacker"

MODULES_FILE="moduli.txt"
TMP_DIR_NAME="tmp/"
NUM_SYSTEM=16
RESULT_FILE="result.txt"


echo "[Step 1] Compiling file...";
g++ solution.cpp -lgmp -o $OUTPUT_EXE_FILE;
echo;

echo "[Step 2] Creating directory for tmp files...";
mkdir $TMP_DIR_NAME;
echo;

echo "[Step 3] Running...";
./$OUTPUT_EXE_FILE                  \
    --modules-file=$MODULES_FILE    \
    --tmp-dir=$TMP_DIR_NAME         \
    --num-system=$NUM_SYSTEM        \
    --result-file=$RESULT_FILE      \
;
echo;

echo "[Step 4] Output result:";
cat $RESULT_FILE;
echo;

echo "[Step 5.1] Deleting executable file...";
rm $OUTPUT_EXE_FILE;
echo "[Step 5.2] Deleting tmp files...";
rm -r -d $TMP_DIR_NAME;
echo;

echo "[Step 6] Checking...";
python3 checker.py              \
    --result $RESULT_FILE       \
    --num-system $NUM_SYSTEM    \
;
echo;
