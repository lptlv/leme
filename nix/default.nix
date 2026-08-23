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
  xwayland,
  libxkbcommon,
  libxcb-wm,
  libinput,
  pixman,

  libdrm,
  libGL,
  libgbm,
  libdisplay-info,
  libliftoff,
  libxcb-render-util,
  libxcb-errors,
  vulkan-loader,
  glslang,
  lcms2,
  seatd,
  hwdata
}:
let
  wlrgit = fetchgit {
    url = "https://gitlab.freedesktop.org/wlroots/wlroots.git";
    rev = "0.20.2";
    hash = "sha256-VdYymvzYp6/R255AK20j4xTd+JbCZgNiRfgeRJD+UZY=";
    fetchSubmodules = false;
  };

  patchedWlr = stdenv.mkDerivation {
    pname = "wlroots";
    version = "0.20.2";

    src = wlrgit;

    patches = [
      ../subprojects/packagefiles/wlroots-rounded.patch
    ];
  };
in
stdenv.mkDerivation {
    pname = "leme";
    version = "git";

    src = fetchFromGitHub {
        owner = "ernestoCruz05";
        repo = "leme";
        rev = "main";
        hash = "sha256-Mzv1KjyipN0P+ZTj9YirRV2f0+MdG7cvuxoYZZaDstY=";
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
      patchedWlr
      xwayland
      libxkbcommon
      libxcb-wm
      libinput
      pixman

      libdrm
      libGL
      libgbm
      libdisplay-info
      libliftoff
      libxcb-render-util
      libxcb-errors
      vulkan-loader
      glslang
      lcms2
      seatd
      hwdata
    ];

    mesonFlags = [
      "-Deffects=true"
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
