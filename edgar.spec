Summary: 2D Platform Game
Name: %{name}
Version: %{version}
Release: %{release}
Source: %{name}-%{version}-%{release}.tar.gz
Vendor: Parallel Realities
Packager: Richard Sweeney
URL: http://www.parallelrealities.co.uk/p/legend-of-edgar.html
Group: Games/Arcade
License: GPL
%description
The Legend of Edgar. When his father fails to return home after venturing
out one stormy night, Edgar sets off on a quest to rescue him.

%prep
%setup -q

%build
make VERSION=%{version} RELEASE=%{release}

%install
make install DESTDIR=$RPM_BUILD_ROOT

%clean

%post


%files
%dir /usr/games
%dir /usr/share/games/%{name}
%dir /usr/share/doc/%{name}
/usr/games/%{name}
/usr/share/games/%{name}/%{name}.pak
/usr/share/doc/%{name}/*
/usr/share/applications/edgar.desktop
/usr/share/icons/hicolor/16x16/apps/%{name}.png
/usr/share/icons/hicolor/32x32/apps/%{name}.png
/usr/share/icons/hicolor/48x48/apps/%{name}.png
/usr/share/icons/hicolor/64x64/apps/%{name}.png
/usr/share/man/man6/%{name}.6
