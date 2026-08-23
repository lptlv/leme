{
  lib,
  stdenv,
  fetchFromGitHub,
  fetchgit,
  pkg-config,
  meson,
  ninja,
  wayland,
  wayland-protocols,
  wayland-scanner,
  wlroots_0_20,
  xwayland,
  libxkbcommon,
  libxcb-wm,
  libinput,
  pixman
}:
let
  wlrootsSrc = fetchgit {
    url = "https://gitlab.freedesktop.org/wlroots/wlroots.git";
    rev = "0.20.2";
    hash = "sha256-VdYymvzYp6/R255AK20j4xTd+JbCZgNiRfgeRJD+UZY=";
    fetchSubmodules = false;
  };
in
stdenv.mkDerivation {
    pname = "leme";
    version = "0.1.0";

    src = builtins.path {
        path = ../.;
        name = "source";
    };

    nativeBuildInputs = [
      wayland-scanner
      pkg-config
      meson
      ninja
    ];

    buildInputs = [
      wayland
      wayland-protocols
      wlroots_0_20
      xwayland
      libxkbcommon
      libxcb-wm
      libinput
      pixman
    ];

    preConfigure = ''
      mkdir -p subprojects/wlroots
      cp -r ${wlrootsSrc}/* subprojects/wlroots/
      chmod -R u+w subprojects/wlroots
    '';

    mesonFlags = [
      "-Ddefault_library=shared"
      "-Deffects=true"
    ];

    passthru = {
        providedSessions = [ "leme" ];
    };

    meta = with lib; {
        description = "Leme, kinda sounds like Lemm from Hollow Knight.";
        homepage = "https://github.com/ernestoCruz05/leme";
        license = licenses.gpl3;
        platforms = platforms.linux;
    };
}
