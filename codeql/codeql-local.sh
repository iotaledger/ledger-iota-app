#! /bin/bash
cd /app
git clone https://github.com/iotaledger/ledger-iota-app -b feat/stardust+shimmer
cd ledger-iota-app
git submodule update --init --recursive
codeql database create ledger-iota-app --language=cpp --command="make"
codeql database analyze ledger-iota-app codeql/cpp-queries:codeql-suites/cpp-security-and-quality.qls --format=sarif-latest --output=/app/output/output.sarif
# annoying fix
sed 's|https://json.schemastore.org/sarif-2.1.0.json|https://raw.githubusercontent.com/oasis-tcs/sarif-spec/master/Schemata/sarif-schema-2.1.0.json|g' -i /app/output/output.sarif
echo "Report written to /app/ledger-iota-app/output.sarif"
