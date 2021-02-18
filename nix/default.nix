with import <nixpkgs> {};
mkShell {
    name = "bic-dev";
    buildInputs = [ gcc gnumake ];
}
