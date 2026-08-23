self:
{
  config,
  lib,
  pkgs,
  ...
}:
let
  cfg = config.programs.leme;
in
{
  # in case this gets added to nixpkgs, uncomment:
  # disabledModules = [ "programs/wayland/leme.nix" ];

  options = {
    programs.leme = {
      enable = lib.mkEnableOption "Leme, kinda sounds like Lemm from Hollow Knight.";
      addLoginEntry = lib.mkOption {
        type = lib.types.bool;
        default = true;
        description = "Whether to add a login entry to your display manager for Leme.";
      };
      package = lib.mkOption {
        type = lib.types.package;
        default = self.packages.${pkgs.stdenv.hostPlatform.system}.leme;
        description = "The leme package to use";
      };
    };
  };

  config = lib.mkIf cfg.enable {
    environment.systemPackages = [
      cfg.package
    ];

    xdg.portal = {
      enable = lib.mkDefault true;

      config = {
        leme = {
          default = [
            "gtk"
          ];
          "org.freedesktop.impl.portal.ScreenCast" = [ "wlr" ];
          "org.freedesktop.impl.portal.Screenshot" = [ "wlr" ];
        };
      };
      extraPortals = with pkgs; [
        xdg-desktop-portal-wlr
        xdg-desktop-portal-gtk
      ];
      wlr.enable = lib.mkDefault true;
      configPackages = [ cfg.package ];
    };

    security.polkit.enable = lib.mkDefault true;
    programs.xwayland.enable = lib.mkDefault true;

    services = {
      displayManager.sessionPackages = lib.mkIf cfg.addLoginEntry [ cfg.package ];
      graphical-desktop.enable = lib.mkDefault true;
    };
  };
}
