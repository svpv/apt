# hey Emacs, its -*- mode: rpm-spec; coding: cyrillic-cp1251; -*-

Name: apt
Version: 0.5.5cnc1
Release: alt3

Summary: Debian's Advanced Packaging Tool with RPM support
Summary(ru_RU.CP1251): Debian APT - Усовершенствованное средство управления пакетами с поддержкой RPM
License: GPL
Group: System/Configuration/Packaging
Packager: APT Development Team <apt@packages.altlinux.org>

Source0: http://moin.conectiva.com.br/files/AptRpm/attachments/%name-%version.tar.bz2
Source1: apt.conf
Source2: genbasedir
Source3: README.rsync
Source4: apt.8
Source5: apt.ru.po

Patch1: apt-0.5.5cnc1-alt-distro.patch
#Patch2: apt-0.5.4cnc1-alt-INLINEDEPFLAG.patch
Patch3: apt-0.5.4cnc1-alt-configure-build.patch
Patch4: apt-0.5.4cnc9-alt-getsrc.patch
Patch5: apt-0.5.4cnc3-alt-md5hash-debug.patch
Patch6: apt-0.5.4cnc9-alt-rsync.patch
Patch7: apt-0.5.4cnc8-alt-rpm_cmd.patch
Patch8: apt-0.5.4cnc8-alt-rpm-fancypercent.patch
#Patch9: apt-0.5.4cnc9-alt-algo.patch
#Patch10: apt-0.5.4cnc9-alt-replace.patch
#Patch11: apt-0.5.4cnc9-alt-fixes.patch
#Patch12: apt-0.5.4cnc9-alt-CachedMD5.patch
Patch13: apt-0.5.5cnc1-alt-pkgpriorities.patch
Patch14: apt-0.5.4cnc9-alt-bz2.patch
#Patch15: apt-0.5.4cnc9-alt-rename-segfault.patch
Patch16: apt-0.5.4cnc9-alt-rpmlistparser-kernel.patch
Patch17: apt-0.5.4cnc9-alt-packagemanager-CheckRConflicts.patch
Patch18: apt-0.5.4cnc9-alt-install_virtual.patch
Patch19: apt-0.5.4cnc9-alt-specialchars.patch
#Patch20: apt-0.5.4cnc9-alt-rpmrecords_epoch.patch
#Patch21: apt-0.5.4cnc9-alt-lockfix.patch
Patch22: apt-0.5.4cnc9-alt-install_virtual_version.patch
Patch23: apt-0.5.5cnc1-alt-methods_gpg_homedir.patch
Patch24: apt-0.5.4cnc9-alt-pkgorderlist_score.patch
#Patch25: apt-0.5.4cnc9-alt-cdrom-unmount.patch
Patch26: apt-0.5.5cnc1-alt-APT_DOMAIN.patch
Patch27: apt-0.5.5cnc1-mattdm-manbuild.patch
Patch28: apt-0.5.5cnc1-alt-debsystem.patch

# Need to be adopted. Or not?
Patch31: apt-0.3.19cnc53-stelian-apt-pkg-algorithms-scores.patch
# Normally not applied, but useful.
Patch32: apt-0.5.4cnc9-alt-getsrc-debug.patch
# Obsolete.
Patch33: apt-0.5.4cnc3-alt-select-genlist.patch

PreReq: %__subst
Requires: libapt = %version-%release
Requires: %{get_dep rpm}, /etc/apt/pkgpriorities, apt-conf, gnupg, alt-gpgkeys
Obsoletes: apt-0.5

BuildPreReq: librpm-devel >= 4.0.4, rpm-build >= 4.0.4

# Automatically added by buildreq on Wed Feb 12 2003
BuildRequires: bzlib-devel docbook-dtds docbook-utils gcc-c++ libdb2-devel libpopt-devel librpm-devel libstdc++-devel openjade perl-SGMLSpm sgml-common xml-common zlib-devel

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

%package -n libapt-devel
Summary: Development files and documentation for APT's core libs
Summary(ru_RU.CP1251): Файлы и документация для разработчиков, использующих библиотеки APT
Group: Development/C
Requires: libapt = %version-%release
Obsoletes: libapt-0.5-devel

%package -n libapt-devel-static
Summary: Development static library for APT's libs
Summary(ru_RU.CP1251): Статическая библиотека APT для разработчиков, использующих библиотеки APT
Group: Development/C
Requires: libapt-devel = %version-%release
Obsoletes: libapt-0.5-devel-static

%package utils
# Analoguous to rpm-build subpackage.
Summary: Utilities to create APT repositaries (the indices)
Summary(ru_RU.CP1251): Утилиты для построения APT-репозитариев (индексов)
Group: Development/Other
Requires: %name = %version-%release, mktemp >= 1:1.3.1, getopt
Requires: %{get_dep rpm}, gnupg, sed
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
%setup -q -n apt-%version
%patch1 -p1
#%patch2 -p1
%patch3 -p1
%patch4 -p1
%patch5 -p1
%patch6 -p1
%patch7 -p1
%patch8 -p1
#%patch9 -p1
#%patch10 -p1
#%patch11 -p1
#%patch12 -p1
%patch13 -p1
%patch14 -p1
#%patch15 -p1
%patch16 -p1
%patch17 -p1
%patch18 -p1
%patch19 -p1
#%patch20 -p1
#%patch21 -p1
%patch22 -p1
%patch23 -p1
%patch24 -p1
#%patch25 -p1
%patch26 -p1
%patch27 -p1
%patch28 -p1

# Need to be adopted. Or not?
#%patch31 -p1
# Turn it on only if you want to see the debugging messages:
#%patch32 -p1 -b .getsrc-debug
# Obsolete.
#%patch33 -p1

%__install -p -m644 %SOURCE3 %SOURCE4 .
%__install -p -m644 %SOURCE5 po/ru.po

%build
%add_optflags -fno-exceptions -D_GNU_SOURCE
pushd doc
	[ -f apt.8 ] || %__install -p -m644 %SOURCE4 .
popd

# Fix gettextization.
if [ ! -s po/LINGUAS ]; then
	%__sed -ne 's/^ALL_LINGUAS="\([^"]*\)"$/\1/p' configure.in >>po/LINGUAS
	%__subst 's/^\(ALL_LINGUAS=\)/#\1/g' configure.in
fi

# Fix url.
%__subst -p 's,/usr/share/common-licenses/GPL,/usr/share/license/GPL,' COPYING

libtoolize --copy --force
aclocal -I buildlib
autoconf

%configure --with-proc-multiply=1 --with-procs=%__nprocs

find -type f -print0 |
	xargs -r0 %__grep -EZl '/var(/lib)?/state/apt' -- |
	xargs -r0 %__subst 's,/var\(/lib\)\?/state/apt,%_localstatedir/apt,g' --

%make clean

%make_build STATICLIBS=1 NOISY=1

%install
%__mkdir_p $RPM_BUILD_ROOT{%_bindir,%_libdir/apt,%_mandir/man{1,5,8},%_includedir/apt-pkg,%_includedir/apt-inst,%_sysconfdir/apt/{vendors.list.d,apt.conf.d}}
%__mkdir_p $RPM_BUILD_ROOT%_localstatedir/apt/lists/partial
%__mkdir_p $RPM_BUILD_ROOT/var/cache/apt/{archives/partial,gen{pkg,src}list}

%__cp -a bin/lib*.so* $RPM_BUILD_ROOT%_libdir/
%__cp -a bin/lib*.a* $RPM_BUILD_ROOT%_libdir/

%__install -p -m755 bin/methods/* $RPM_BUILD_ROOT%_libdir/apt/
%__install -p -m755 bin/apt-* $RPM_BUILD_ROOT%_bindir/
%__install -p -m755 bin/{%name-*,*list} %SOURCE2 $RPM_BUILD_ROOT%_bindir/

%__install -p -m644 apt-pkg/{,*/}*.h $RPM_BUILD_ROOT%_includedir/apt-pkg/
%__install -p -m644 apt-inst/{,*/}*.h $RPM_BUILD_ROOT%_includedir/apt-inst/

%__install -p -m644 doc/*.1 $RPM_BUILD_ROOT%_man1dir/
%__install -p -m644 doc/*.5 $RPM_BUILD_ROOT%_man5dir/
%__install -p -m644 doc/*.8 $RPM_BUILD_ROOT%_man8dir/

%__install -p -m644 %SOURCE1 $RPM_BUILD_ROOT%_sysconfdir/apt/

%__ln_s -f rsh $RPM_BUILD_ROOT%_libdir/apt/ssh
%__ln_s -f gzip $RPM_BUILD_ROOT%_libdir/apt/bzip2

%__mkdir_p $RPM_BUILD_ROOT%_datadir
%__rm -rf $RPM_BUILD_ROOT%_datadir/locale
%__cp -a locale $RPM_BUILD_ROOT%_datadir/

%define docdir %_docdir/%name-%version
%__mkdir_p $RPM_BUILD_ROOT%docdir
%__cp -a README* TODO COPYING AUTHORS* doc/examples \
	$RPM_BUILD_ROOT%docdir/

%find_lang --output=%name.lang \
	apt libapt-pkg3.3

%triggerun -- apt < 0.5.4
CONF=/etc/apt/apt.conf
if [ -s "$CONF" ]; then
	%__subst 's/HoldPkgs/Hold/;s/AllowedDupPkgs/Allow-Duplicated/;s/IgnorePkgs/Ignore/;s/PostInstall/Post-Install/;s|Methods .*|Methods "/usr/lib/apt";|;s|PubringPath *"\([^"]*\)"|Pubring "\1/pubring.gpg"|g' "$CONF"
fi

%post -n libapt -p %post_ldconfig
%postun -n libapt -p %postun_ldconfig

%files -f %name.lang
%_bindir/apt-*
%_libdir/apt
%exclude %_libdir/apt/rsync
%dir %_sysconfdir/apt
%config(noreplace) %_sysconfdir/apt/apt.conf
%dir %_sysconfdir/apt/apt.conf.d
%dir %_sysconfdir/apt/vendors.list.d
%_mandir/man?/*
%docdir
%_localstatedir/apt

%defattr(2770,root,rpm,2770)
%_cachedir/apt/archives

%files utils
%_bindir/*
%exclude %_bindir/apt-*

%defattr(2770,root,rpm,2770)
%_cachedir/apt/gen*list

%files -n libapt
%_libdir/*.so.*

%defattr(2770,root,rpm,2770)
%dir %_cachedir/apt

%files -n libapt-devel
%_libdir/*.so
%_includedir/*

%files -n libapt-devel-static
%_libdir/*.a

%files rsync
%dir %_libdir/apt
%_libdir/apt/rsync

%changelog
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

* Wed Jun 10 2002 Ivan Zakharyaschev <imz@altlinux.ru> 0.3.19cnc55-alt6
- fixes in genbasedir from apt-utils:
  + srclist generation: now it correctly includes the information on produced
    binaries (the list was empty before), and also correctly treats --mapi
    (the fixed bug caused misbehaviour of 'apt-get source', in the "2nd branch
    of DoSource()" as referenced by me in previous changelogs);
  + --flat option.

* Mon Jun 09 2002 Ivan Zakharyaschev <imz@altlinux.ru> 0.3.19cnc55-alt5
- renamed the 'build' subpackage to 'utils' (because there is already
  an 'apt-build' project at freshmeat.net).

* Mon Jun 09 2002 Ivan Zakharyaschev <imz@altlinux.ru> 0.3.19cnc55-alt4
- separate subpkg 'build': analoguous to rpm-build, contains teh utilities to
  generate indices (as a consequence less dependencies of 'apt' pkg,
  little size);
- fix the work of 'apt-get source' for source packages with multiple binaries
  with help of feedback and debugging patches from Anton Denisov
  <fire@kgpu.kamchatka.ru> (only the first branch of control flow in DoSource()
  is fixed, but that should be enough for normal use).

* Tue Mar 26 2002 Dmitry V. Levin <ldv@alt-linux.org> 0.3.19cnc55-alt3
- Added librpm-4.0.4 build support.
- Built with librpm-4.0.4, updated buildrequires.

* Thu Mar 21 2002 Dmitry V. Levin <ldv@alt-linux.org> 0.3.19cnc55-alt2
- Added kernel-aureal and NVIDIA_kernel to default AllowedDupPkgs.
- Updated patch for pkgRPMPM::ExecRPM.
- Reenabled rsh method.
- Updated rpmpriorities.
- fixed genbasedir patch.
- Set explicit Packager tag.
- Dropped obsolete trigger.
- lib%name: Conflicts: %name < %%version-%%release.
- Renamed patches.

* Wed Mar 13 2002 Alexander Bokovoy <ab@altlinux.ru> 0.3.19cnc55-alt1
- apt-0.3.19cnc55 integrated
- Fixed:
    + rpmpm-exec_rpm patch
    + genbasedir
- Removed:
    + rpmpm-nodeps patch (already upstream)
    + rsh method (already upstream)

* Mon Dec 10 2001 Dmitry V. Levin <ldv@alt-linux.org> 0.3.19cnc53-alt6
- Fixed rpm --nodeps option usage in pkgRPMPM::ExecRPM (#0000215).

* Fri Nov 23 2001 Dmitry V. Levin <ldv@alt-linux.org> 0.3.19cnc53-alt5
- Applied scoring algorithm patch (Stelian Pop <stelian.pop@fr.alcove.com>)
- Updated package requires.

* Mon Nov 19 2001 Dmitry V. Levin <ldv@alt-linux.org> 0.3.19cnc53-alt4
- Dropped outdated pofile (already upstream).
- Corrected "Executing RPM" message generation.

* Fri Nov 16 2001 Dmitry V. Levin <ldv@alt-linux.org> 0.3.19cnc53-alt3
- Updated patches: genbasedir, configure-db3, i18n.

* Wed Nov 15 2001 Alexander Bokovoy <ab@altlinux.ru> 0.3.19cnc53-alt2
+ apt-0.3.19cnc53-2cl integrated. Most of our patches moved to upstream
- Fixed (from Conectiva's changelog):
    + fixed bug in mirror patch
    + cleaned up gen{pkg,src}list (Alexander Bokovoy <ab@altlinux.ru
    + fixed crash bug in genpkglist
    + configure.in patch to detect rpmdb (Stelian Pop <stelian.pop@fr.alcove.com>)
    +       * Skips correctly over empty package directories
    +       * Adds the --bz2only argument which makes genbasedir
    +       to generate only the .bz2 compressed versions of pkglist
    +       and srclist (space gain...)
    +       * Doesn't change the timestamps on pkglists/srclists if
    +       the contents are not modified (making possible for example
    +       to make several consecutive runs of genbasedir without
    +       having the apt clients download the indexes again and again).
    +       * Some minor cleanups (remove the temporary files in /tmp
    +       at the end of the script etc).
    + (Stelian Pop <stelian.pop@fr.alcove.com>)
    + cleanup patch for gensrclist (Stelian Pop <stelian.pop@fr.alcove.com>)
    + fixed multi-arch pkg handling (Stelian Pop <stelian.pop@fr.alcove.com>)
    + updated russian translation (Alexander Bokovoy <ab@altlinux.ru>
    + updated i18n (Dmitry Levin <ldv@alt-linux.org>)
    + replaced Apt::GPG::Pubring with Apt::GPG::PubringPath
      also uses --homedir instead of --keyring in gpg
      (Alexander Bokovoy <ab@altlinux.ru>)
    + patch to detect replaced (instead of just removed) packages
      Dmitry Levin <ldv@alt-linux.org>
    + updated mirrors patch
    + Fixed mirrors infinite loop bug (closes: #4420).
    + Fixed error handling.
- Added:
    + added kernel-tape to default AllowedDupPkgs
    + added patch to fix bug in genbasedir with empty dirs
      (Stelian Pop <stelian.pop@fr.alcove.com>)
    + added -K (RPM::Check-Signatures) option to verify rpm sigs
    + Added mirrors patch.

* Fri Nov 02 2001 Dmitry V. Levin <ldv@alt-linux.org> 0.3.19cnc52-alt6
- Initial build with rpm4.

* Thu Oct 04 2001 Dmitry V. Levin <ldv@altlinux.ru> 0.3.19cnc52-alt5
- Fixed i18n support and probably smth else
  (configure.in, config.h.in and i18n.h were broken, kojima sux).
- Implemented 18n for apt-pkg/rpm, updated POTFILES.in and russian translation.

* Mon Oct 01 2001 Dmitry V. Levin <ldv@altlinux.ru> 0.3.19cnc52-alt4
- Removed "^(kernel|alsa)[0-9]*-headers" from RPM::HoldPkgs list.
- Added (experimental) replaced packages support.

* Fri Sep 07 2001 Ivan Zakharyaschev <imz@altlinux.ru> 0.3.19cnc52-alt3.1
- apt-cdrom fix (patch 6) reworked: apt-cdrom used to not detect a second
  list file (srclist) if both (pkglist and srclist) were present in one
  directory on the disk (in non-thorough mode); hopefully now it works how
  we expect it to do.

* Thu Aug 09 2001 Dmitry V. Levin <ldv@altlinux.ru> 0.3.19cnc52-alt3
- Libification.
- Reworked compilation options again: we add only '-fno-exceptions' now.

* Tue Aug 07 2001 Dmitry V. Levin <ldv@altlinux.ru> 0.3.19cnc52-alt2
- More code cleanup in tools/gen{pkg,src}list.cc.
- Added %%optflags_nocpp to compilation options.

* Mon Aug 06 2001 Alexander Bokovoy <ab@altlinux.ru> 0.3.19cnc52-alt1
- cnc52, gcc 3 and Solaris fixes
- RPM4 check is disabled for the moment
- File method fix has been integrated into mainstream
- RPM::RemoveOptions, RPM::UpdateOptions have been added
- Generation of Package list fixed

* Thu Aug 02 2001 Dmitry V. Levin <ldv@altlinux.ru> 0.3.19cnc51-alt4
- Added trigger for better apt-conf-* migration.

* Tue Jul 31 2001 Dmitry V. Levin <ldv@altlinux.ru> 0.3.19cnc51-alt3
- Updated:
  + rpmpriorities,
  + AllowedDupPkgs,
  + HoldPkgs.
- Moved *.list to apt-conf-* packages.

* Fri Jul 20 2001 Alexander Bokovoy <ab@altlinux.ru> 0.3.19cnc51-alt2
- Fixed:
  + Bug in file method which prevented authentication from working correctly

* Fri Jul 20 2001 Alexander Bokovoy <ab@altlinux.ru> 0.3.19cnc51-alt1
- cnc51

* Wed Jun 27 2001 Alexander Bokovoy <ab@avilink.net> 0.3.19cnc46-alt2
- cnc46
- kernel(|24)-{headers,source} added to HoldPkgs
- REPOSITORIO-APT-HOWTO added (Portugal)
- AllowedDupPackages -> AllowedDupPkgs

* Thu Jun 07 2001 Dmitry V. Levin <ldv@altlinux.ru> 0.3.19cnc38-alt4
- Various fixes in %_bindir/genbasedir.

* Thu May 17 2001 Dmitry V. Levin <ldv@altlinux.ru> 0.3.19cnc38-alt3
- Fixed build.
- Updated rpmpriorities.

* Mon Apr 16 2001 Alexander Bokovoy <ab@avilink.net> 0.3.19cnc38-alt2
- More duplicate packages from kernel series allowed.

* Sun Apr 15 2001 Alexander Bokovoy <ab@avilink.net> 0.3.19cnc38-alt1
- cnc38
- Updated:
  + apt-cdrom now works correctly
  + default architecture has been changed to i586
  + ssh method as wrapper to rsh one

* Mon Mar 19 2001 Alexander Bokovoy <ab@avilink.net> 0.3.19cnc37-ipl3mdk
- Updated:
  + New patch for genbasedir to allow pass default key to GnuPG

* Sun Mar 18 2001 Dmitry V. Levin <ldv@altlinux.ru> 0.3.19cnc37-ipl2mdk
- Fixed:
  + Build/installation of manpages without yodl sources;
  + Uncompressed small patches.
- Updated:
  + AllowedDupPackages list according to new kernel naming scheme;
  + URLs to use new domain name.

* Sat Mar 17 2001 Alexander Bokovoy <ab@avilink.net> 0.3.19cnc37-ipl1mdk
- Fixed:
    + APT::GPG::Pubring renamed to APT::GPG::PubringPath
    + Pubring support patch changed to use --homedir instead of --keyring
- Updated:
    + APT cnc37
    + Fingerprint and repository sources changed to reflect ALT Linux
      new public key ring

* Mon Feb 19 2001 Alexander Bokovoy <ab@avilink.net> 0.3.19cnc36-ipl4mdk
- Sisyphus source repository added to sources.list

* Mon Feb 19 2001 Alexander Bokovoy <ab@avilink.net> 0.3.19cnc36-ipl3mdk
- New version

* Fri Feb 16 2001 Alexander Bokovoy <ab@avilink.net> 0.3.19cnc35-ipl2mdk
- Static library compilation added
- Static library goes to apt-devel-static due sizes of library

* Thu Feb 15 2001 Alexander Bokovoy <ab@avilink.net> 0.3.19cnc35-ipl1mdk
- New version
- Rsh method from upstream apt-get ported
- Spec file follows libification of rpm now

* Mon Jan 22 2001 Alexander Bokovoy <ab@avilink.net> 0.3.19cnc32-ipl4mdk
- New upstream version
- Russian translation updated
- Rebuild with new RPM library

* Mon Jan 22 2001 Alexander Bokovoy <ab@avilink.net> 0.3.19cnc28-ipl1mdk
- New upstream version
- cnc28 still lacks correct GNUPG checking code, our patch is neccessary.
- Genbasedir slightly patched again.

* Sun Jan 21 2001 Alexander Bokovoy <ab@avilink.net> ipl11mdk
- Typo in methods/gpg.cc fixed.

* Sun Jan 21 2001 Alexander Bokovoy <ab@avilink.net> ipl10mdk
- APT::GPG::Pubring option to specify default gnupg public ring added to check
  distributors' signs automatically via default pubring in RPM package
- ftp user password changed to IPL one.

* Tue Jan 09 2001 Dmitry V. Levin <ldv@fandra.org> 0.3.19cnc27-ipl7mdk
- Specfile cleanup.

* Mon Jan 08 2001 Alexander Bokovoy <ab@avilink.net> ipl5mdk
- genbasedir help message fixed

* Mon Jan 08 2001 Alexander Bokovoy <ab@avilink.net> ipl4mdk
- Integration with main IPL package:
- Russian translation for command line messages added
- Russian translation of package summary/description

* Thu Jan 04 2001 AEN <aen@logic.ru>
- Real Sisyphus URL & IPLabs Fingerptint added
- changed rpmpriorities

* Thu Jan 04 2001 AEN <aen@logic.ru>
- build for RE

* Tue Dec 12 2000 Frederic Lepied <flepied@mandrakesoft.com> 0.3.19cnc27-1mdk
- first mandrake version.

* Thu Dec 07 2000 Andreas Hasenack <andreas@conectiva.com>
- damn! Wrong URL in sources.list, atualizacoes.conectiva.com
  doesn't exist, of course...

* Thu Dec 07 2000 Andreas Hasenack <andreas@conectiva.com>
- updated sources.list with new mirrors and new download tree
- removed (noreplace) for the sources.list file for this
  upgrade. It will be easier for the user. The (noreplace)
  should be back in place after this update as we expect no
  further big modifications for that file, only new mirrors.

* Wed Dec 06 2000 Alfredo K. Kojima <kojima@conectiva.com.br>
- fixed prob in vendors.list

* Tue Dec 05 2000 Alfredo K. Kojima <kojima@conectiva.com.br>
- released version 0.3.19cnc27

* Wed Nov 08 2000 Alfredo K. Kojima <kojima@conectiva.com.br>
- released version 0.3.19cnc26

* Mon Nov 06 2000 Alfredo K. Kojima <kojima@conectiva.com.br>
- released version 0.3.19cnc25

* Thu Nov 02 2000 Alfredo K. Kojima <kojima@conectiva.com.br>
- released version 0.3.19cnc24

* Thu Nov 02 2000 Rud<E1> Moura <ruda@conectiva.com>
- updated source.list (again)

* Thu Nov 02 2000 Rud<E1> Moura <ruda@conectiva.com>
- updated source.list

* Wed Nov 01 2000 Alfredo K. Kojima <kojima@conectiva.com.br>
- released version 0.3.19cnc23
- added cache directories for gen{pkg,src}list
- pt_BR manpages

* Tue Oct 31 2000 Alfredo K. Kojima <kojima@conectiva.com.br>
- released version 0.3.19cnc22
- Requires -> PreReq in apt-devel

* Mon Oct 30 2000 Alfredo Kojima <kojima@conectiva.com>
- collapsed libapt-pkg-devel and -doc to apt-devel

* Mon Oct 30 2000 Alfredo K. Kojima <kojima@conectiva.com.br>
- released version 0.3.19cnc21

* Sun Oct 29 2000 Alfredo K. Kojima <kojima@conectiva.com.br>
- released version 0.3.19cnc20

* Sun Oct 29 2000 Alfredo K. Kojima <kojima@conectiva.com.br>
- released version 0.3.19cnc19
- added gensrclist
- support for apt-get source

* Fri Oct 27 2000 Alfredo K. Kojima <kojima@conectiva.com.br>
- released version 0.3.19cnc18

* Thu Oct 26 2000 Alfredo K. Kojima <kojima@conectiva.com.br>
- released version 0.3.19cnc17
- new manpages

* Wed Oct 25 2000 Alfredo K. Kojima <kojima@conectiva.com.br>
- released version 0.3.19cnc16

* Sun Oct 22 2000 Alfredo K. Kojima <kojima@conectiva.com.br>
- released version 0.3.19cnc15

* Sat Oct 21 2000 Alfredo K. Kojima <kojima@conectiva.com.br>
- released version 0.3.19cnc14

* Thu Oct 19 2000 Claudio Matsuoka <claudio@conectiva.com>
- new upstream release: 0.3.9cnc13

* Tue Oct 17 2000 Eliphas Levy Theodoro <eliphas@conectiva.com>
- added rpmpriorities to filelist and install

* Tue Oct 17 2000 Claudio Matsuoka <claudio@conectiva.com>
- updated to 0.3.19cnc12
- fresh CVS snapshot including: support to Acquire::ComprExtension,
  debug messages removed, fixed apt-cdrom, RPM DB path, rpmlib call
  in pkgRpmLock::Close(), package priority kludge removed, i18n
  improvements, and genbasedir/genpkglist updates.
- handling language setting in genpkglist to make aptitude happy

* Wed Oct 11 2000 Alfredo K. Kojima <kojima@conectiva.com>
- released version 0.3.19cnc11
- fixed problem with shard lib symlinks

* Tue Oct 10 2000 Alfredo K. Kojima <kojima@conectiva.com>
- released version 0.3.19cnc10

* Mon Oct  2 2000 Claudio Matsuoka <claudio@conectiva.com>
- fixed brown paper bag bug with method permissions
- added parameter --sign to genbasedir
- added html/text doc files

* Sat Sep 30 2000 Claudio Matsuoka <claudio@conectiva.com>
- bumped to 0.3.19cnc9
- added vendors.list
- added gpg method
- fixed minor stuff to make Aptitude work
- added missing manpages
- fixed shared libs
- split in apt, libapt-pkg, libapt-pkg-devel, libapt-pkg-doc
- rewrote genbasedir in shell script (original was in TCL)
- misc cosmetic changes

* Tue Sep 26 2000 Alfredo K. Kojima <kojima@conectiva.com>
- released version 0.3.19cnc8

* Wed Sep 20 2000 Alfredo K. Kojima <kojima@conectiva.com>
- released version 0.3.19cnc7

* Mon Sep 18 2000 Alfredo K. Kojima <kojima@conectiva.com>
- released version 0.3.19cnc6

* Sat Sep 16 2000 Alfredo K. Kojima <kojima@conectiva.com>
- released version 0.3.19cnc5

* Fri Sep 15 2000 Alfredo K. Kojima <kojima@conectiva.com>
- released version 0.3.19cnc4

* Mon Sep 12 2000 Alfredo K. Kojima <kojima@conectiva.com>
- released version 0.3.19cnc3

* Mon Sep 5 2000 Alfredo K. Kojima <kojima@conectiva.com>
- renamed package to apt, with version 0.3.19cncV

* Mon Sep 5 2000 Alfredo K. Kojima <kojima@conectiva.com>
- 0.10
- added genpkglist and rapt-config
- program names changed back to apt-*

* Mon Sep 4 2000 Alfredo K. Kojima <kojima@conectiva.com>
- 0.9

* Mon Sep 4 2000 Alfredo K. Kojima <kojima@conectiva.com>
- 0.8

* Mon Sep 4 2000 Alfredo K. Kojima <kojima@conectiva.com>
- 0.7

* Fri Sep 1 2000 Alfredo K. Kojima <kojima@conectiva.com>
- fixed typo in sources.list

* Tue Aug 31 2000 Alfredo K. Kojima <kojima@conectiva.com>
- version 0.6

* Tue Aug 31 2000 Alfredo K. Kojima <kojima@conectiva.com>
- version 0.5

* Tue Aug 31 2000 Alfredo K. Kojima <kojima@conectiva.com>
- version 0.4

* Wed Aug 30 2000 Alfredo K. Kojima <kojima@conectiva.com>
- version 0.3

* Thu Aug 28 2000 Alfredo K. Kojima <kojima@conectiva.com>
- second try. new release with direct hdlist handling

* Thu Aug 10 2000 Alfredo K. Kojima <kojima@conectiva.com>
- initial package creation. Yeah, it's totally broken for sure.

# Local Variables:
# compile-command: "rpmbuild --target=i586 -ba apt.spec"
# End:
