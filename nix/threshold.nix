{
  stdenv,
  pkg-config,
  wiringpi,
  lib,
  ...
}:
stdenv.mkDerivation {
  pname = "temper_thresh_c";
  version = "0.0.1";
  src = ../c;
  nativeBuildInputs = [
    pkg-config
  ];
  buildInputs = [ wiringpi ];
  makeFlags = [ "temper_thresh" ];
  installPhase = ''
    install -Dm755 temper_thresh $out/bin/temper_thresh
  '';
  meta = {
    mainProgram = "temper_thresh";
    description = "";
    license = lib.licenses.gpl3Plus;
  };
}
