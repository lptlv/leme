{
  lib,
  stdenv,
  fetchFromGitHub,
  fetchFromGitLab,
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
  wlrootsSrc = fetchFromGitLab {
    domain = "gitlab.freedesktop.org";
    owner = "wlroots";
    repo = "wlroots";
    rev = "0.20.2";
    hash = "sha256-VdYymvzYp6/R255AK20j4xTd+JbCZgNiRfgeRJD+UZY=";
  };
in
stdenv.mkDerivation {
  pname = "leme";
  version = "git";

  src = builtins.path {
        path = ../.;
        name = "source";
    };

  patches = [ ../subprojects/packagefiles/wlroots-rounded.patch ];

  postPatch = ''
    mkdir subprojects/wlroots
    cp -r ${wlrootsSrc}/* subprojects/wlroots
    chmod -R u+w subprojects/wlroots
    patch -d subprojects/wlroots -p1 \
      < subprojects/packagefiles/wlroots-rounded.patch
  '';

  nativeBuildInputs = [
     wayland-scanner
     pkg-config
     meson
     ninja
  ];

  buildInputs = [
      wayland
      wayland-protocols
      wlrootsSrc
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
  ];

  passthru.providedSessions = [ "leme" ];
}
