#! /bin/bash
cd /app/ledger-iota-app
mkdir codeql/output
make clean

codeql database create codeql/ledger-iota-app --language=cpp --command="make"
codeql database analyze codeql/ledger-iota-app codeql/cpp-queries:codeql-suites/cpp-security-and-quality.qls --format=sarif-latest --output=/app/ledger-iota-app/codeql/output/output.sarif
# annoying fix
sed 's|https://json.schemastore.org/sarif-2.1.0.json|https://raw.githubusercontent.com/oasis-tcs/sarif-spec/master/Schemata/sarif-schema-2.1.0.json|g' -i /app/ledger-iota-app/codeql/output/output.sarif
echo "Report written to /app/ledger-iota-app/output.sarif"
