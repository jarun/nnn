%global debug_package %{nil}

Name:		nnn
Version:	1.0
Release:	1%{?dist}
Summary:	The missing terminal file browser for X 

Group:		Applications/Engineering
License:	GPLv3
URL:		https://github.com/jarun/nnn
Source0:	%{name}-%{version}.tar.gz

BuildRequires:	gcc binutils make gzip
BuildRequires:	ncurses-devel readline-devel

%description
nnn is a fork of noice, a blazing-fast lightweight terminal file browser with easy keyboard shortcuts for navigation, opening files and running tasks. noice is developed considering terminal based systems. There is no config file and mime associations are hard-coded. However, the incredible user-friendliness and speed make it a perfect utility on modern distros.

nnn can use the default desktop opener at runtime. It also comes with nlay - a customizable bash script to handle media types. It adds new navigation options, enhanced DE integration, a disk usage analyzer mode, comprehensive file details and much more. Add to that a huge performance boost. For a detailed comparison, visit nnn vs. noice.

If you want to edit a file in vim with some soothing music in the background while referring to a spec in your GUI PDF viewer, nnn got it! All from the same terminal session. Follow the instructions in the quickstart section and see how nnn simplifies those long desktop sessions...

%prep
%setup -q


%build
make %{?_smp_mflags}

%install
%{__install} -m755 -d %{buildroot}%{_bindir}
%{__install} -m755 -d %{buildroot}%{_mandir}/man1
%{__install} -m755 nnn %{buildroot}%{_bindir}/nnn
%{__install} -m755 nlay %{buildroot}%{_bindir}/nlay
%{__install} -m644 nnn.1 %{buildroot}%{_mandir}/man1/nnn.1

%files
%{_bindir}/nnn
%{_bindir}/nlay
%{_mandir}/man1/nnn.1.gz
%doc README.md
%doc LICENSE
%doc CHANGELOG


%changelog
* Wed Apr 26 2017 Michael Fenn <michaelfenn87@gmail.com> - 1.0-1
- Initial RPM
