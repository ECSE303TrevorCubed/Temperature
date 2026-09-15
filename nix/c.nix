{
  stdenv,
  pkg-config,
  wiringpi,
  lib,
  ...
}:
stdenv.mkDerivation {
  pname = "temper_c";
  version = "0.0.1";
  src = ../c;
  nativeBuildInputs = [
    pkg-config
  ];
  buildInputs = [ wiringpi ];
  meta = {
    mainProgram = "temper";
    description = "";
    license = lib.licenses.gpl3Plus;
  };
}
