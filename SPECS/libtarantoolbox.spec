%bcond_with static

%define _prefix /usr/local

%define __libiprotocluster_version 20130905.1741
%define __libiprotoclusterdevel_version 20130905.1741

Name:           libtarantoolbox
Version:        %{__version}
Release:        1%{?dist}
Summary:        tarantool/octopus box C client library

Group:          Development/Libraries
License:        BSD
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-buildroot
BuildRequires:  git
BuildRequires:  gcc cmake
BuildRequires:  libiprotocluster-devel >= %{__libiprotoclusterdevel_version}
BuildRequires:  libiprotocluster >= %{__libiprotocluster_version}
Requires:       libiprotocluster >= %{__libiprotocluster_version}

%description
tarantool/octopus box C client library. Built from revision %{__revision}

%prep
%setup -n iproto/tarantool/box

%build
%cmake %{?with_static:-DBUILD_SHARED_LIBS=OFF} .
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT
%if ! %{with static}
mkdir -p $RPM_BUILD_ROOT%{_sysconfdir}/ld.so.conf.d
echo "%{_libdir}" > $RPM_BUILD_ROOT%{_sysconfdir}/ld.so.conf.d/%{name}.conf
%endif

%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%if ! %{with static}
%files
%{_libdir}/*.so
%{_sysconfdir}/ld.so.conf.d/%{name}.conf
%endif

%package devel
Summary:  tarantool/octopus box C client library header files
Group:    Development/Libraries
Requires: %{name} = %{version}-%{release}
Requires: libiprotocluster-devel >= %{__libiprotoclusterdevel_version}

%description devel
tarantool/octopus box C client library header files. Built from revision %{__revision}

%files devel
%{_includedir}/*

%package static
Summary: tarantool/octopus box C client library static libraries
Group:   Development/Libraries

%description static
tarantool/octopus box C client library static libraries. Built from revision %{__revision}

%if %{with static}
%files static
%{_libdir}/*.a
%endif

%changelog
* Mon Dec 10 2012 Aleksey Mashanov <a.mashanov@corp.mail.ru>
- Initial release
