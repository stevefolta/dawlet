{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
	buildInputs = [ pkgs.alsaLib.dev ];
	}
