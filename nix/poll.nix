{
  stdenv,
  pkg-config,
  wiringpi,
  lib,
  ...
}:
stdenv.mkDerivation {
  pname = "temper_poll_c";
  version = "0.0.1";
  src = ../c;
  nativeBuildInputs = [
    pkg-config
  ];
  buildInputs = [ wiringpi ];
  makeFlags = [ "temper_poll" ];
  installPhase = ''
    install -Dm755 temper_poll $out/bin/temper_poll
  '';
  meta = {
    mainProgram = "temper_poll";
    description = "";
    license = lib.licenses.gpl3Plus;
  };
}
