# hey Emacs, its -*- mode: rpm-spec; coding: cyrillic-cp1251; -*-

Name: apt
Version: 0.5.15cnc6
Release: alt5

Summary: Debian's Advanced Packaging Tool with RPM support
Summary(ru_RU.CP1251): Debian APT - Усовершенствованное средство управления пакетами с поддержкой RPM
License: GPL
Group: System/Configuration/Packaging
Url: https://moin.conectiva.com.br/AptRpm
Packager: APT Development Team <apt@packages.altlinux.org>

Source0: http://moin.conectiva.com.br/files/AptRpm/attachments/%name-%version.tar.bz2
Source1: apt.conf
Source2: genbasedir
Source3: README.rsync
Source4: apt.ru.po
Source5: ChangeLog-rpm.old

Patch9: apt-0.5.15cnc6-alt-aclocal-warnings.patch
Patch10: apt-0.5.15cnc5-alt-libtool.patch
Patch11: apt-0.5.15cnc6-alt-fixes.patch
Patch12: apt-0.5.15cnc5-alt-tinfo.patch
Patch13: apt-0.5.15cnc5-alt-rpm-build.patch
Patch14: apt-0.5.15cnc5-alt-distro.patch
Patch15: apt-0.5.15cnc5-alt-debsystem.patch
Patch16: apt-0.5.15cnc6-alt-defaults.patch
Patch17: apt-0.5.5cnc5-alt-rsync.patch
Patch18: apt-0.5.15cnc5-alt-getsrc.patch
Patch19: apt-0.5.15cnc5-alt-parseargs.patch
Patch20: apt-0.5.15cnc5-alt-rpm_cmd.patch
Patch21: apt-0.5.15cnc6-alt-rpm-fancypercent.patch
Patch22: apt-0.5.15cnc5-alt-methods_gpg_homedir.patch
Patch23: apt-0.5.15cnc5-alt-md5hash-debug.patch
Patch24: apt-0.5.15cnc5-alt-packagemanager-CheckRConflicts.patch
Patch25: apt-0.5.5cnc4.1-alt-fixpriorsort.patch
Patch26: apt-0.5.4cnc9-alt-pkgorderlist_score.patch
Patch27: apt-0.5.15cnc6-alt-virtual_scores.patch
Patch28: apt-0.5.15cnc5-alt-system-lua5.patch
Patch29: apt-0.5.15cnc5-alt-findrepos.patch
Patch30: apt-0.5.15cnc5-alt-gettext.patch
Patch31: apt-0.5.15cnc6-alt-rpm-order.patch
Patch32: apt-0.5.15cnc6-alt-pkgcachegen.patch
Patch33: apt-0.5.15cnc6-alt-apt-shell.patch

# Normally not applied, but useful.
Patch101: apt-0.5.4cnc9-alt-getsrc-debug.patch

PreReq: %__subst
Requires: libapt = %version-%release
Requires: rpm >= 4.0.4-alt28, /etc/apt/pkgpriorities, apt-conf, gnupg, alt-gpgkeys
Obsoletes: apt-0.5

# new rpmlib.
BuildPreReq: librpm-devel >= 4.0.4-alt28

# for docs.
BuildPreReq: docbook-utils

# lua5.
BuildPreReq: liblua5-devel

# autopoint
BuildPreReq: cvs

%def_disable static
%{?_enable_static:BuildPreReq: glibc-devel-static}

# all the rest.
BuildPreReq: gcc-c++ libreadline-devel libstdc++-devel libtinfo-devel

%define risk_usage_en This package is still under development.

%description
A port of Debian's APT tools for RPM based distributions,
or at least for Conectiva. It provides the apt-get utility that
provides a simpler, safer way to install and upgrade packages.
APT features complete installation ordering, multiple source
capability and several other unique features.

%risk_usage_en

%define risk_usage Данный пакет пока еще находится в стадии разработки.

%description -l ru_RU.CP1251
Перенесенные из Debian средства управления пакетами APT, включающие
в себя поддержку RPM, выполненную компанией Conectiva (Бразилия).
Этот пакет содержит утилиту apt-get для простой и надежной установки
и обновления пакетов. APT умеет автоматически разрешать зависимости
при установке, обеспечивает установку из нескольких источников и
целый ряд других уникальных возможностей.

%risk_usage

%package -n libapt
Summary: APT's core libraries
Group: System/Libraries
Obsoletes: libapt-0.5
PreReq: librpm >= 4.0.4-alt28

%package -n libapt-devel
Summary: Development files and documentation for APT's core libs
Summary(ru_RU.CP1251): Файлы и документация для разработчиков, использующих библиотеки APT
Group: Development/C
Requires: libapt = %version-%release, librpm-devel >= 4.0.4-alt28
Obsoletes: libapt-0.5-devel

%package -n libapt-devel-static
Summary: Development static library for APT's libs
Summary(ru_RU.CP1251): Статическая библиотека APT для разработчиков, использующих библиотеки APT
Group: Development/C
Requires: libapt-devel = %version-%release, librpm-devel-static >= 4.0.4-alt28
Obsoletes: libapt-0.5-devel-static

%package utils
# Analoguous to rpm-build subpackage.
Summary: Utilities to create APT repositaries (the indices)
Summary(ru_RU.CP1251): Утилиты для построения APT-репозитариев (индексов)
Group: Development/Other
Requires: %name = %version-%release, mktemp >= 1:1.3.1, getopt
Requires: gnupg, sed
Obsoletes: apt-0.5-utils

%package rsync
Summary: rsync method support for APT
Summary(ru_RU.CP1251): Поддержка метода rsync для APT
Group: Development/Other
Requires: %name = %version-%release, rsync >= 2.5.5-alt3
Obsoletes: apt-0.5-rsync

%description -n libapt
This package contains APT's package manipulation library,
modified for RPM.

%risk_usage_en

%description -n libapt-devel
This package contains the header files and libraries for developing with
APT's package manipulation library, modified for RPM.

%risk_usage_en

%description -n libapt-devel-static
This package contains static libraries for developing with APT's
package manipulation library, modified for RPM.

%risk_usage_en

%description utils
This package contains the utility programs that can prepare a repositary of
RPMS binary and source packages for future access by APT (by generating
the indices).

It relates to 'apt' package analoguously to how 'rpm' relates to 'rpm-build' package.

%risk_usage_en

%description rsync
This package contains method 'rsync' for APT.

%risk_usage_en

%description -n libapt -l ru_RU.CP1251
В этом пакете находится библиотеки управления пакетами
из комплекта APT. В отличие от оригинальной версии для Debian, этот
пакет содержит поддержку для формата RPM.

%risk_usage

%description -n libapt-devel -l ru_RU.CP1251
В этом пакете находятся заголовочные файлы и библиотеки для разработки
программ, использующих библиотеки управления пакетами
из комплекта APT. В отличие от оригинальной версии для Debian, этот
пакет содержит поддержку для формата RPM.

%risk_usage

%description -n libapt-devel-static -l ru_RU.CP1251
В этом пакете находятся статические библиотеки для разработки программ,
использующих библиотеки управления пакетами из
комплекта APT. В отличие от оригинальной версии для Debian, этот пакет
содержит поддержку для формата RPM.

%risk_usage

%description utils -l ru_RU.CP1251
В этом пакете находятся программы-утилиты, которые могут репозитарий
бинарных и исходных пакетов RPM приготовить для доступа с помощью APT
(сгенерировать индексы).

Он относится к пакету 'apt' аналогично тому, как 'rpm'к 'rpm-build'.

%risk_usage

%description rsync -l ru_RU.CP1251
В этом пакете находится метод 'rsync' для APT

%risk_usage

%prep
%setup -q
%patch9 -p1
%patch10 -p1
%patch11 -p1
%patch12 -p1
%patch13 -p1
%patch14 -p1
%patch15 -p1
%patch16 -p1
%patch17 -p1
%patch18 -p1
%patch19 -p1
%patch20 -p1
%patch21 -p1
%patch22 -p1
%patch23 -p1
%patch24 -p1
%patch25 -p1
%patch26 -p1
%patch27 -p1
%patch28 -p1
%patch29 -p1
%patch30 -p1
%patch31 -p1
%patch32 -p1
%patch33 -p1

# Use system-wide lua5
pushd lua
# keep only local/ and lua/
%__rm -rfv include lib luac *.[ch]
popd

# Turn it on only if you want to see the debugging messages:
#%patch101 -p1 -b .getsrc-debug

%__install -p -m644 %SOURCE3 %SOURCE5 .
%__install -p -m644 %SOURCE4 po/ru.po

%build
# Fix url.
%__subst -p 's,/usr/share/common-licenses/GPL,/usr/share/license/GPL,' COPYING

autoreconf -fisv -I buildlib

# --disable-dependency-tracking Speeds up one-time builds
%configure --includedir=%_includedir/apt-pkg %{subst_enable static}

# Probably this obsolete now?
find -type f -print0 |
	xargs -r0 %__grep -EZl '/var(/lib)?/state/apt' -- |
	xargs -r0 %__subst -p 's,/var\(/lib\)\?/state/apt,%_localstatedir/%name,g' --
%make_build

%install
%__mkdir_p %buildroot%_sysconfdir/%name/{%name.conf,sources.list,vendors.list}.d
%__mkdir_p %buildroot%_libdir/%name/scripts
%__mkdir_p %buildroot%_localstatedir/%name/{lists/partial,prefetch}
%__mkdir_p %buildroot%_cachedir/%name/{archives/partial,gen{pkg,src}list}

%makeinstall includedir=%buildroot%_includedir/apt-pkg

%__install -p -m755 %SOURCE2 %buildroot%_bindir/
%__install -p -m644 %SOURCE1 %buildroot%_sysconfdir/%name/

# This is still needed.
%__ln_s -f rsh %buildroot%_libdir/%name/methods/ssh
%__ln_s -f gzip %buildroot%_libdir/%name/methods/bzip2

# Cleanup
find %buildroot%_includedir -type f -name rpmshowprogress.h -print -delete
%__rm -f %buildroot%_libdir/*.la

%__bzip2 -9fk ChangeLog-rpm.old

%find_lang %name

unset RPM_PYTHON

%triggerun -- apt < 0:0.5.4
CONF=/etc/apt/apt.conf
if [ -s "$CONF" ]; then
	%__subst 's/HoldPkgs/Hold/;s/AllowedDupPkgs/Allow-Duplicated/;s/IgnorePkgs/Ignore/;s/PostInstall/Post-Install/;s|Methods .*|Methods "/usr/lib/apt/methods";|;s|PubringPath *"\([^"]*\)"|Pubring "\1/pubring.gpg"|g' "$CONF"
fi

%triggerun -- apt < 0:0.5.15cnc5-alt2
CONF=/etc/apt/apt.conf
if [ -s "$CONF" ]; then
	%__subst -p 's,"/usr/lib/apt","/usr/lib/apt/methods",g' "$CONF"
fi

%post -n libapt -p %post_ldconfig
%postun -n libapt -p %postun_ldconfig

%files -f %name.lang
%_bindir/apt-*
%_libdir/%name
%exclude %_libdir/%name/methods/rsync
%dir %_sysconfdir/%name
%config(noreplace) %_sysconfdir/%name/%name.conf
%dir %_sysconfdir/%name/*.d
%_mandir/man?/*
%_localstatedir/%name
%doc README* TODO COPYING AUTHORS* ChangeLog-rpm.old.bz2 doc/examples contrib

%defattr(2770,root,rpm,2770)
%_cachedir/%name/archives

%files utils
%_bindir/*
%exclude %_bindir/apt-*

%defattr(2770,root,rpm,2770)
%_cachedir/%name/gen*list

%files -n libapt
%_libdir/*.so.*

%defattr(2770,root,rpm,2770)
%dir %_cachedir/%name

%files -n libapt-devel
%_libdir/*.so
%_includedir/*

%if_enabled static
%files -n libapt-devel-static
%_libdir/*.a
%endif

%files rsync
%dir %_libdir/%name
%dir %_libdir/%name/methods
%_libdir/%name/methods/rsync
# Probably %%doc with README.rsync?

%changelog
* Mon Jul 05 2004 Kachalov Anton <mouse@altlinux.ru> 0.5.15cnc6-alt5
- apt-shell fixes (#3091)

* Mon Jun 07 2004 Dmitry V. Levin <ldv@altlinux.org> 0.5.15cnc6-alt4
- apt-shell fixes from Mouse (#4306).

* Sat May 15 2004 Dmitry V. Levin <ldv@altlinux.org> 0.5.15cnc6-alt3
- apt-pkg/pkgcachegen.cc:
  Remove old sources cache file before creating new one.
- More fixes reported by compiler, patch by Anton V. Denisov.

* Fri May 14 2004 Dmitry V. Levin <ldv@altlinux.org> 0.5.15cnc6-alt2
- Fixed aclocal warnings, patch by Anton V. Denisov.
- Updated russian translation from Anton Denisov.

* Thu May 13 2004 Kachalov Anton <mouse@altlinux.ru> 0.5.15cnc6-alt1
- Updated to 0.5.15cnc6.
- New:
  + apt-0.5.15cnc6-alt-rpm-order (fix RPM::Order default value)
- Updated:
  + apt-0.5.15cnc6-alt-fixes
  + apt-0.5.15cnc6-alt-defaults
  + apt-0.5.15cnc6-alt-rpm-fancypercent
  + apt-0.5.15cnc6-alt-virtual_scores
- Merged upstream:
  + apt-0.5.15cnc6-alt-install_virtual

* Fri Feb 27 2004 Dmitry V. Levin <ldv@altlinux.org> 0.5.15cnc5-alt4
- Fixed build with fresh autotools.

* Wed Jan 21 2004 Dmitry V. Levin <ldv@altlinux.org> 0.5.15cnc5-alt3
- Readded contrib to documentation.
- Updated russian translation from Anton Denisov.

* Mon Jan 19 2004 Dmitry V. Levin <ldv@altlinux.org> 0.5.15cnc5-alt2
- Added one more %%triggerun to correct apt.conf,
  due to apt methods migration.
- Applied patch from Alexey Tourbin to use systemwide lua5.
- Applied patch from Sviatoslav Sviridov to workaround VendorList bug.

* Fri Jan 16 2004 Dmitry V. Levin <ldv@altlinux.org> 0.5.15cnc5-alt1
- Specfile cleanup.
- Rediffed patches.
- Fixed --help/--version segfault.
- Fixed some compilation warnings.
- Relocated methods to %_libdir/%name/methods/.

* Thu Jan 15 2004 Dmitry V. Levin <ldv@altlinux.org> 0.5.15cnc5-alt0.3
- Updated:
  + apt-0.5.15cnc5-alt-rpm
  + apt-0.5.15cnc5-alt-rpm-fancypercent

* Tue Jan 13 2004 Anton Kachalov <mouse@altlinux.ru> 0.5.15cnc5-alt0.2
- Updated and applied:
  + apt-0.5.15cnc5-alt-install_virtual
  + apt-0.5.15cnc5-alt-virtual_scores
- Removed:
  + apt-0.5.4cnc9-alt-install_virtual_version (merged upstream).

* Fri Dec 26 2003 Anton V. Denisov <avd@altlinux.org> 0.5.15cnc5-alt0.1
- Updated to 0.5.15cnc4.
- Updated alt-rpm.patch.

* Tue Dec 09 2003 Anton V. Denisov <avd@altlinux.org> 0.5.15cnc4-alt0.2
- Updated and applied alt-getsrc.patch.
- Following patches still stay unapplied:
  + alt-install_virtual.patch
  + alt-install_virtual_version.patch
  + alt-virtual_scores.patch
  
* Mon Dec 08 2003 Anton V. Denisov <avd@altlinux.org> 0.5.15cnc4-alt0.1
- Updated to 0.5.15cnc4.
- Updated alt-distro.patch.
- Updated russian translation.
- Get rid of %_libdir/*.la files.
- Still have disabled patches.

* Tue Nov 25 2003 Anton V. Denisov <avd@altlinux.org> 0.5.15cnc3-alt0.1
- Updated to 0.5.15cnc3
- Temporary disabled patches:
  + apt-0.5.4cnc9-alt-getsrc.patch
  + apt-0.5.5cnc4-alt-install_virtual.patch
  + apt-0.5.5cnc4.1-alt-virtual_scores.patch
- Merged upstream patches:
  + apt-0.5.15cnc1-upstream-pinning-fix.patch
- NOTE:
  + this release can not be used with hasher (until we 'll
    update temporary disabled patches);
  + all packages which uses libapt need to be rebuilt (library version
    changed).
- TODO:
  + check and update our patches (most important);
  + update russian translation.

* Tue Nov 11 2003 Anton V. Denisov <avd@altlinux.org> 0.5.15cnc1-alt0.2
- added apt-0.5.15cnc1-upstream-pinning-fix.patch.
- Updated russian translation.

* Sat Nov 08 2003 Anton V. Denisov <avd@altlinux.org> 0.5.15cnc1-alt0.1
- Updated to 0.5.15cnc1, renumbered patches.
- Removed patches:
  + apt-0.5.5cnc5-panu-nodigest.patch
- Updated patches:
  + alt-distro.patch
  + alt-rpm_cmd.patch
  + alt-defaults.patch
- Merged upstream patches:
  + alt-bz2.patch
  + alt-versionmatch.patch
  + alt-versionmatch2.patch
  + alt-cleanups.patch
  + apt-0.5.12_apt-get_backports.patch
  + apt-0.5.12_apt-cache_backports.patch
  + apt-0.5.12_doc_backports.patch
  + apt-0.5.12_methods_backports.patch
  + apt-0.5.12_apt-pkg_backports.patch
  + apt-0.5.12_acqprogress_backports.patch
  + apt-0.5.12_tests_backports.patch
  + apt-0.5.14_apt_get_and_docs.patch
- Temporary disabled patches:
  + alt-install_virtual_version.patch (need to be updated by author)
- Following patches wasn't accepted by upstream (what to do with it?):
  + alt-packagemanager-CheckRConflicts.patch
  + alt-debsystem.patch
  + alt-install_virtual.patch
  + alt-fixpriorsort.patch
  + alt-rpm-fancypercent.patch
  + alt-virtual_scores.patch
  + alt-parseargs.patch

* Thu Oct 09 2003 Anton V. Denisov <avd@altlinux.org> 0.5.5cnc6-alt0.8
- added BuildRequires: python (fix build in hasher).
- partial merge with upstream APT (0.5.14):
  + apt-0.5.14_apt_get_and_docs.patch

* Wed Oct 08 2003 Anton V. Denisov <avd@altlinux.org> 0.5.5cnc6-alt0.7
- added apt-0.5.5cnc6-alt-cleanups.patch (fixed some compiler warnings)
- sync with apt-0.5.5cnc4.1-alt7:
  + updated alt-fixpriorsort.patch
  + updated alt-rpm-fancypercent.patch
  + updated alt-parseargs.patch (and update it for cnc6)
  + added alt-virtual_scores.patch
  + renamed spec file.

* Tue Oct 07 2003 Anton V. Denisov <avd@altlinux.org> 0.5.5cnc6-alt0.5
- partial merge with upstream APT (0.5.12):
  + apt-0.5.12_apt-cache_backports.patch
  + apt-0.5.12_apt-get_backports.patch
  + apt-0.5.12_doc_backports.patch
  + apt-0.5.12_methods_backports.patch
  + apt-0.5.12_apt-pkg_backports.patch
  + apt-0.5.12_acqprogress_backports.patch
  + apt-0.5.12_tests_backports.patch

* Mon Oct 06 2003 Anton V. Denisov <avd@altlinux.org> 0.5.5cnc6-alt0.4
- sync with apt-0.5.5cnc4.1-alt6:
  + added apt-0.5.5cnc4.1-alt-parseargs.patch
  + added apt-0.5.5cnc6-alt-fixpriorsort.patch

* Fri Sep 05 2003 Anton V. Denisov <avd@altlinux.org> 0.5.5cnc6-alt0.3
- s/$RPM_BUILD_ROOT/%%buildroot/g in spec file.
- Updated BuildRequires in Sisyphus (20030704) env.
- Updated russian translation.
- sync with apt-0.5.5cnc4.1-alt5:
  + added apt-0.5.5cnc4-alt-defaults.patch
  + cleaned up /etc/apt.conf file.
  + updated alt-install_virtual.patch
  + added apt-0.5.5cnc4-alt-versionmatch2.patch
  + do not build static libraries by default.

* Mon Jun 16 2003 Anton V. Denisov <avd@altlinux.org> 0.5.5cnc6-alt0.1
- Updated to 0.5.5cnc6.
- Re-added patches due to upstream changes:
  + alt-debsystem.patch (apt-pkg/deb/debsystem.cc was added again).
- Downgraded patches due to upstream changes:
  + alt-getsrc.patch (apt-pkg/deb/debsrcrecords.h was added again).
- Packaged directories:
  + Dir::Bin::scripts (/usr/lib/apt/scripts);
- Updated %%doc (contrib dir added).

* Fri Jun 06 2003 Anton V. Denisov <avd@altlinux.org> 0.5.5cnc5-alt0.5
- old changelog entries come back as rpm_old_changelog.
- updated %%find_lang call.
- added -I m4 for %%__aclocal call.
- more macroszification.
- My Mother birthday edition.

* Tue Jun 03 2003 Anton V. Denisov <avd@altlinux.org> 0.5.5cnc5-alt0.4
- %%install section reworked (%%makeinstall use).
- %%build section cleaned up.
- %%_libdir/*.la moved to libapt-devel subpackage.
- %name.8 removed from sources.
- Dropped obsolete old patches.
- Dropped old changelog entries.

* Tue Jun 03 2003 Anton V. Denisov <avd@altlinux.org> 0.5.5cnc5-alt0.3
- Updated to 0.5.5cnc5, renumbered patches.
- Added patches:
  + apt-0.5.5cnc5-panu-nodigest.patch
- Updated patches:
  + alt-getsrc
  + alt-rsync
  + alt-tinfo
  + alt-rpm (check this one)
- Removed patches due to upstream changes:
  + alt-debsystem
- Merged upstream patches:
  + alt-i18n-apt-cdrom
  + apt-cnc-20030322-algo
- Updated russian translation.
- Explicitly use autoconf-2.5 and automake-1.7 for build.
- Major changes in spec file (need more changes).
- Updated buildrequires (libbeecrypt-devel and glibc-devel-static added).

* Mon Apr 28 2003 Dmitry V. Levin <ldv@altlinux.org> 0.5.5cnc4.1-alt4
- apt-get: substitute virtual package with real one (mouse).
- libapt: ignore serial and check both for version and
  version-release while matching version string (mouse).
- Mouse birthday edition.

* Tue Mar 25 2003 Dmitry V. Levin <ldv@altlinux.org> 0.5.5cnc4.1-alt3
- Applied cnc-20030322 algorithm changes.
- Updated russian translation (avd).

* Tue Mar 11 2003 Dmitry V. Levin <ldv@altlinux.org> 0.5.5cnc4.1-alt2
- Fixed -lncurses/-ltinfo linkage.
- Updated buildrequires.

* Sat Mar 08 2003 Dmitry V. Levin <ldv@altlinux.org> 0.5.5cnc4.1-alt1
- Updated to 0.5.5cnc4.1

* Fri Mar 07 2003 Dmitry V. Levin <ldv@altlinux.org> 0.5.5cnc4-alt1
- Updated to 0.5.5cnc4, renumbered patches.
- Updated patches:
  + alt-rpm_cmd
- Removed patches due to upstream fixes:
  + alt-APT_DOMAIN
  + alt-specialchars
- Packaged directories:
  + Dir::Etc::sourceparts (/etc/apt/sources.list.d);
  + Dir::State::prefetch (/var/lib/apt/prefetch).
- apt-cdrom: i18n'ed (avd).
- Updated russian translation (avd).

* Fri Feb 28 2003 Dmitry V. Levin <ldv@altlinux.org> 0.5.5cnc3-alt1
- Updated to 0.5.5cnc3

* Sun Feb 23 2003 Dmitry V. Levin <ldv@altlinux.org> 0.5.5cnc2-alt1
- Updated to 0.5.5cnc2
- Merged upstream patches:
  + alt-rpmlistparser-kernel
  + mattdm-manbuild
- Updated genbasedir (svd).
- Cleaned up genbasedir a bit.
- Cleaned up and updated buildrequires.

* Thu Feb 13 2003 Dmitry V. Levin <ldv@altlinux.org> 0.5.5cnc1-alt3
- Introduced APT::Ignore-dpkg support and set this flag by default,
  to address #0002119.

* Wed Feb 12 2003 Dmitry V. Levin <ldv@altlinux.org> 0.5.5cnc1-alt2
- Updated russian translation (Vyacheslav Dikonov).
- Updated buildrequires.

* Fri Feb 07 2003 Dmitry V. Levin <ldv@altlinux.org> 0.5.5cnc1-alt1
- Updated to 0.5.5cnc1
- Fixed build:
  + alt-APT_DOMAIN
  + mattdm-manbuild
- Merged upstream patches:
  + alt-algo
  + alt-replace
  + alt-fixes
  + alt-CachedMD5
  + alt-rename-segfault
  + alt-rpmrecords_epoch
  + alt-lockfix
  + alt-cdrom-unmount
- Updated patches:
  + alt-distro
  + alt-pkgpriorities
  + alt-methods_gpg_homedir
- Removed patches:
  + alt-INLINEDEPFLAG

* Tue Jan 28 2003 Dmitry V. Levin <ldv@altlinux.org> 0.5.4cnc9-alt8
- apt-cdrom: Unmout cdrom in case if package file wasn't found (avd).
- apt-cdrom: Fixed default disk name (#0001886).

* Tue Jan 21 2003 Dmitry V. Levin <ldv@altlinux.org> 0.5.4cnc9-alt7
- apt-pkg: fixed RPMTAG_EPOCH handling (#0002019, avd).
- apt-get: try to fix lock problem (#0002013, vsu).
- apt-pkg: added APT::Install::VirtualVersion support (mouse).
- methods/gpg (#0001945):
  + added APT::GPG::Homedir support and enabled it by default;
  + dropped APT::GPG::Pubring support.
- apt-pkg: experimental patch for pkgOrderList::Score (#0001931).

* Fri Jan 17 2003 Dmitry V. Levin <ldv@altlinux.org> 0.5.4cnc9-alt6
- apt: PreReq: %__subst (#0001801).
- apt-get: added APT::Install::Virtual support (mouse).
- apt-cdrom: applied alt-specialchars patch from Anton V. Denisov,
  needs to be tested though.
- apt.conf: added "-adv" and "-linus" kernels to Allow-Duplicated list.

* Thu Dec 26 2002 Dmitry V. Levin <ldv@altlinux.org> 0.5.4cnc9-alt5
- apt-pkg/packagemanager.cc (pkgPackageManager::CheckRConflicts):
  Ignore versionless reverse dependencies (mouse).

* Mon Dec 23 2002 Dmitry V. Levin <ldv@altlinux.org> 0.5.4cnc9-alt4
- Removed builtin defaults for RPM::Allow-Duplicated and RPM::Hold options
  (was added in 0.5.4cnc9-alt1).
- rpmListParser::Package(): removed "kernel" hack.

* Thu Dec 19 2002 Sviatoslav Sviridov <svd@altlinux.ru> 0.5.4cnc9-alt3
- patch for check .bz2 extension in file method
- fixed possible segfault in pkgAcquire::Item::Rename

* Tue Dec 17 2002 Sviatoslav Sviridov <svd@altlinux.ru> 0.5.4cnc9-alt2
- Updated rsync method:
  + Fixed bug leading to race condition.
  + Acquire::rsync::options:: in apt.config allows specification
    of any user-defined option for /usr/bin/rsync.
  + Support port specification in URIs
- Updated README.rsync

* Tue Dec 03 2002 Dmitry V. Levin <ldv@altlinux.org> 0.5.4cnc9-alt1
- Reverted 1 of 4 hunks in sorting order fix, to be compatible with upstream.
  We will use pkgpriorities instead.
- Renamed rpmpriorities to pkgpriorities and moved it to apt-conf package.
- Several compilation fixes.
- Fixed gettextization.
- Set builtin defaults for RPM::Allow-Duplicated and RPM::Hold options.
- Renumbered patches.
- Replaced patch to genbasedir with shell script.
- genbasedir:
  + Added new options: --no-oldhashfile, --newhashfile, --no-newhashfile.
  + Enabled generation of both old and new hashfiles by default.
- Do not use __progname in CachedMD5 implementation.
- Fixed apt upgrade trigger.
- Renamed to apt and built for Sisyphus.

* Mon Dec 02 2002 Kachalov Anton <mouse@altlinux.ru> 0.5.4cnc9-alt0.5
- Fixed replace support

* Tue Nov 26 2002 Dmitry V. Levin <ldv@altlinux.org> 0.5.4cnc9-alt0.4
- Updated genbasedir patch.
- Fixed sorting algorithm (mouse).
- rpmpriorities: removed libs and obsolete packages.

* Sat Nov 23 2002 Ivan Zakharyaschev <imz@altlinux.ru> 0.5.4cnc9-alt0.3
- utils: add -0.5 suffix to %_cachedir/apt/gen* (to enable caching for the
  corresponting utils; <svd@altlinux.ru>);
- describe --mapi in genbasedir usage message;
- include some empty /etc/apt/*.d/ which can be used;

* Sat Nov 16 2002 Sviatoslav Sviridov <svd@altlinux.ru> 0.5.4cnc9-alt0.2
- patch for add option "--fancypercent" to rpm
- patch for genbasedir
- Fixed dependencies:
  + rsync >= 2.5.5-alt3 (now in sisyphus) for apt-0.5-rsync
  + sed for apt-utils (by genbasedir)
- Updated apt.conf:
  + added option RPM::Order="true"

* Mon Oct 28 2002 Ivan Zakharyaschev <imz@altlinux.ru> 0.5.4cnc9-alt0.1
- sync with apt-0.3.19cnc55-alt9:
 + rpmpriorities: updated lists (up to alt9);
 + %_localstatedir/apt: fixed permissions
   (used to be: drwxrws--- root rpm, now: drwxr-xr-x root root);
- apt.conf: APT::GPG::PubringPath -> APT::GPG::Pubring transition
  (the default apt.conf and %%triggerun affected);
  (this is what you have to do to make signed sources work!)
- port getsrc patch (from ALT's apt-0.3 branch);
- new upstream release: apt-0.5.4cnc9 ("early remove" problems are said
  to be solved now).

* Wed Oct 16 2002 Sviatoslav Sviridov <svd@altlinux.ru> 0.5.4cnc8-alt0.1
- new release: apt-0.5.4cnc8

* Tue Sep 17 2002 Svaitoslav Sviridov <svd@altlinux.ru> 0.5.4cnc7-alt0.1
- new release: apt-0.5.4cnc7
- included patch for rsync method

* Sun Aug 11 2002 Anton V. Denisov <avd@altlinux.ru> 0.5.4cnc6-alt0.1
- Updated:
	+ APT-0.5.4cnc6 (bugfix release - fixed some segfaults)

* Wed Aug 07 2002 Anton V. Denisov <avd@altlinux.ru> 0.5.4cnc5-alt0.2
- Fixed:
	+ BuildRequires (updated by new version of buildreq utility)

* Tue Aug 06 2002 Anton V. Denisov <avd@altlinux.ru> 0.5.4cnc5-alt0.1
- Fixed:
	+ Symlinks for ssh and bzip2 methods
	+ Spec file (%name!=apt - I forgot this)
- Updated:
	+ APT-0.5.4cnc5
- Removed:
	+ apt-0.5.4cnc1-alt-enable-rsh-method.patch

* Wed Jul 31 2002 Anton V. Denisov <avd@altlinux.ru> 0.5.4cnc4-alt0.1
- Fixed:
    + apt.conf syntax a little
    + %doc syntax a little
- Updated:
    + APT-0.5.4cnc4
    + BuildRequires
-Removed:
    + apt-0.5.4cnc3-alt-configure-version.patch

* Sat Jul 27 2002 Anton V. Denisov <avd@altlinux.ru> 0.5.4cnc3-alt0.1
- Fixed:
    + libapt-0.5-devel requires
    + apt.conf syntax
- Updated:
    + APT-0.5.4cnc3
    + apt.conf
    + rpmpriorities
    + APT Development Team e-mail
    + apt-0.5 requires
    + select-genlist.patch for new version
    + Spec file
    + %doc section
- Added:
    + Patch for some debug in md5 operations.
    + apt-0.5.4cnc3-alt-configure-version.patch

* Fri Jul 19 2002 Ivan Zakharyaschev <imz@altlinux.ru> 0.5.4cnc1-alt0.3
- make genbasedir-0.5 call gen{pkg,src}list-0.5 respectively

* Fri Jul 19 2002 Ivan Zakharyaschev <imz@altlinux.ru> 0.5.4cnc1-alt0.2
- fix the trigger script;
- add 0.5 to the package names and apt-utils binaries (to make co-existence
  of apt-utils possible).

* Fri Jul 19 2002 Anton V. Denisov <avd@altlinux.ru> 0.5.4cnc1-alt1
- New upstream release.
- Some patches regenerated for new version.
- Spec modified for new version.
- It's a just build for Deadalus - not for actual use.
- I just built it but not test yet.

# Local Variables:
# compile-command: "rpmbuild -ba --target=i586 apt.spec"
# End:
