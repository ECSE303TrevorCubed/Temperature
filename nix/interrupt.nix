{
  stdenv,
  pkg-config,
  wiringpi,
  lib,
  ...
}:
stdenv.mkDerivation {
  pname = "temper_interr_c";
  version = "0.0.1";
  src = ../c;
  nativeBuildInputs = [
    pkg-config
  ];
  buildInputs = [ wiringpi ];
  makeFlags = [ "temper_interr" ];
  installPhase = ''
    install -Dm755 temper_interr $out/bin/temper_interr
  '';
  meta = {
    mainProgram = "temper_interr";
    description = "";
    license = lib.licenses.gpl3Plus;
  };
}
