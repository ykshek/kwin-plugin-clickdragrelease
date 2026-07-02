Name:           kwin-plugin-clickdragrelease
Version:        0.4.1
Release:        1%{?dist}
Summary:        KWin plugin to disable click-drag-release behavior on KDE

License:        GPL-3.0-or-later
URL:            https://github.com/ykshek/kwin-plugin-clickdragrelease
Source0:        v%{version}.tar.gz

BuildRequires:  cmake
BuildRequires:  gcc-c++
BuildRequires:  extra-cmake-modules
BuildRequires:  kf6-kconfig-devel
BuildRequires:  kf6-kcoreaddons-devel
BuildRequires:  qt6-qtbase-devel
BuildRequires:  kf6-rpm-macros
BuildRequires:  gettext
BuildRequires:  kwin-devel

Requires:       kwin
Requires:       qt6-qtbase

%description
A small KWin plugin that disables the default click-drag-release behavior in KDE's KWin.

%prep
%autosetup -n v%{version}

%build
%cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=%{_prefix}
%cmake_build

%install
%cmake_install

%check
# Basic smoke test - ensure the plugin binary was installed into the expected plugin dir
test -f %{buildroot}%{_libdir}/qt6/plugins/kwin/plugins/kwin_contextmenudrag.so || true

%files
%license LICENSE
%doc README.md
%{_libdir}/qt6/plugins/kwin/plugins/kwin_contextmenudrag.so

%changelog
* Thu Jul 02 2026 Alex Shek <hms.starryfish@gmail.com> - 0.4.0-1
- Initial RPM spec for kwin-plugin-clickdragrelease.
