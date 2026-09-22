{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    treefmt-nix.url = "github:numtide/treefmt-nix";
    appimage.url = "github:ralismark/nix-appimage";
  };

  outputs =
    { self, ... }@inputs:
    inputs.flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import inputs.nixpkgs { inherit system; };
        wiringpi' =
          let
            orig = pkgs.wiringpi;
            version = "3.20";
            src = pkgs.fetchFromGitHub {
              owner = "WiringPi";
              repo = "WiringPi";
              rev = "v${version}";
              hash = "sha256-9dOooR8XrhSfR+3g20HLy5kC5mnLHb1D+/uMlcqMPSk=";
            };
            mkSubProject =
              subprj: buildInputs:
              (orig.passthru.mkSubProject {
                inherit subprj src buildInputs;
              }).overrideAttrs
                (old: {
                  version = "3.20";
                  __intentionallyOverridingVersion = old.__intentionallyOverridingVersion or true;
                });
            wiringPi = mkSubProject "wiringPi" [ pkgs.libxcrypt ];
            devLib = mkSubProject "devLib" [ wiringPi ];
            wiringPiD = mkSubProject "wiringPiD" [
              pkgs.libxcrypt
              wiringPi
              devLib
            ];
            gpio = mkSubProject "gpio" [
              pkgs.libxcrypt
              wiringPi
              devLib
            ];
          in
          pkgs.symlinkJoin {
            pname = "wiringpi";
            inherit version;
            paths = [
              wiringPi
              devLib
              wiringPiD
              gpio
            ];
            passthru = {
              inherit src mkSubProject;
              inherit
                wiringPi
                devLib
                wiringPiD
                gpio
                ;
            };
            meta = orig.meta;
          };
        treefmtconfig = inputs.treefmt-nix.lib.evalModule pkgs {
          projectRootFile = "flake.nix";
          programs = {
            alejandra.enable = true;
            yamlfmt.enable = true;
            mdformat = {
              enable = true;
              plugins = ps: [
                ps.mdformat-gfm
              ];
              settings = {
                wrap = 88;
                end-of-line = "lf";
              };
            };
            clang-format.enable = true;
            nixfmt.enable = true;
            typstyle.enable = true;
          };
        };
      in
      {
        formatter = treefmtconfig.config.build.wrapper;
        devShells.default = pkgs.mkShell {
          nativeBuildInputs = with pkgs; [
            clang-tools
            pkg-config
            gcc
          ];

          packages =
            with pkgs;
            [
              typst
              typstyle
              nil
              nixd
            ]
            ++ [
              wiringpi'
            ]
            ++ pkgs.lib.optionals pkgs.stdenv.isLinux [ wiringpi' ];
        };
        packages =
          let
            temper_apps =
              builtins.mapAttrs
                (
                  _: pkg:
                  pkg.overrideAttrs (oldAttrs: {
                    passthru = (oldAttrs.passthru or { }) // {
                      appimage = inputs.appimage.bundlers.${system}.default pkg;
                    };
                  })
                )
                {
                  temper_poll_c = pkgs.callPackage ./nix/poll.nix { wiringpi = wiringpi'; };
                  temper_interr_c = pkgs.callPackage ./nix/interrupt.nix { wiringpi = wiringpi'; };
                  temper_thresh_c = pkgs.callPackage ./nix/threshold.nix { wiringpi = wiringpi'; };
                };
            temper_report = pkgs.callPackage ./nix/report.nix { };
            ci = pkgs.callPackage ./nix/ci.nix {
              apps = temper_apps;
              reports = temper_report;
            };
          in
          {
            default = ci;
            inherit (temper_apps)
              temper_poll_c
              temper_interr_c
              temper_thresh_c
              ;
            inherit temper_report ci;
          };
        checks = {
          formatting = treefmtconfig.config.build.check self;
        };
      }
    );
}
