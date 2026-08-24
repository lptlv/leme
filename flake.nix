{
  description = "Leme, kinda sounds like Lemm from Hollow Knight. As simple and stoic.";

  inputs = {
    nixpkgs.url = "https://channels.nixos.org/nixos-unstable/nixexprs.tar.xz";
  };

  outputs =
    { self, nixpkgs, ... }:
    let
      systems = [
        "x86_64-linux"
        "aarch64-linux"
      ];

      forEachSystem =
        perSystem: nixpkgs.lib.genAttrs systems (system: perSystem nixpkgs.legacyPackages.${system});

      withDefaultPackage =
        module:
        { pkgs, lib, ... }:
        {
          imports = [ module ];
          programs.leme.package = lib.mkDefault self.packages.${pkgs.stdenv.hostPlatform.system}.default;
        };
    in
    {
      overlays.default = final: _: {
        leme = final.callPackage ./nix/default.nix { };
      };
      packages = forEachSystem (pkgs: {
        default = pkgs.callPackage ./nix/default.nix { };
      });

      nixosModules.default = withDefaultPackage ./nix/nixos-modules.nix;
    };
}
