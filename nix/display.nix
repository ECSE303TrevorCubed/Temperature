{
  stdenv,
  pkg-config,
  wiringpi,
  lib,
  ...
}:
stdenv.mkDerivation {
  pname = "temper_display_c";
  version = "0.0.1";
  src = ../c;
  nativeBuildInputs = [
    pkg-config
  ];
  buildInputs = [ wiringpi ];
  makeFlags = [ "temper_display" ];
  installPhase = ''
    install -Dm755 temper_display $out/bin/temper_display
  '';
  meta = {
    mainProgram = "temper_display";
    description = "";
    license = lib.licenses.gpl3Plus;
  };
}
