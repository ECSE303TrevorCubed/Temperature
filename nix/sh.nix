{
  writeShellApplication,
  wiringpi,
  lib,
  ...
}:
writeShellApplication {
  name = "temper";
  text = builtins.readFile ../sh/main.sh;
  runtimeInputs = [
    wiringpi
  ];
  meta = {
    description = "";
    license = lib.licenses.gpl3Plus;
  };
}
