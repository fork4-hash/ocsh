#!/bin/bash

EXEC="./ocsh"
TOTAL=0
PASSED=0
FAILED=0

if [ ! -x "$EXEC" ]; then
    echo "Erreur : executable ./ocsh introuvable."
    exit 1
fi

run_test() {
    NAME="$1"
    COMMAND="$2"
    EXPECTED="$3"

    TOTAL=$((TOTAL + 1))
    RESULT=$(printf '%s\n' "$COMMAND" | timeout 5 "$EXEC" 2>&1)

    if echo "$RESULT" | grep -Fq "$EXPECTED"; then
        echo "[PASS] $NAME"
        PASSED=$((PASSED + 1))
    else
        echo "[FAIL] $NAME"
        FAILED=$((FAILED + 1))
    fi
}

echo "========== TESTS OCSH =========="

run_test "Commande echo" "echo Bonjour" "Bonjour"
run_test "Commande pwd" "pwd" "/"

echo "========== RESULTATS =========="
echo "Total : $TOTAL"
echo "Réussis : $PASSED"
echo "Échoués : $FAILED"
