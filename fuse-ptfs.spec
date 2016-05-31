Name:		fuse-ptfs
Version:	1.0.0
Release:	1%{?dist}
Summary:	Allows mounting partition tables via fuse.
License:	GPL
URL:		https://github.com/joeyhub/fuse-ptfs/
Source0:	fuse-ptfs-%{version}.tgz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires:	fuse-devel
BuildRequires:	parted-devel
BuildRequires:	autoconf
BuildRequires:	automake
Requires:	fuse
Requires:	parted

%description

%prep
%setup -c -q

%build
aclocal
autoconf
automake --add-missing
%configure --prefix=%{_prefix}
make %{?_smp_mflags}

%install
make install DESTDIR=%{buildroot}

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

%files
%defattr (-,root,root)
%{_bindir}/fuse-ptfs

%changelog

