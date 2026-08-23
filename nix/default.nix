{
  lib,
  stdenv,
  fetchFromGitHub,
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

    mesonFlags = [
      "-Ddefault_library=shared"
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
