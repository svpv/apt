# hey Emacs, its -*- rpm-spec -*-
# $Id: apt,v 1.5 2002/03/13 19:01:26 ab Exp $

Name: apt
Version: 0.3.19cnc55
Release: alt3

Summary: Debian's Advanced Packaging Tool with RPM support
Summary(ru_RU.CP1251): Debian APT - ”совершенствованное средство управлени€ пакетами с поддержкой RPM
License: GPL
Group: System/Configuration/Packaging
Packager: APT Development Team <apt@alt-linux.org>

Source0: %name-%version.tar.bz2
Source1: %name.conf
Source2: rpmpriorities

Patch1: %name-0.3.19cnc32-alt-distro.patch
Patch2: %name-0.3.19cnc55-alt-AllowedDupPkgs-HoldPkgs.patch
Patch3: %name-0.3.19cnc52-alt-INLINEDEPFLAG.patch
Patch4: %name-0.3.19cnc53-alt-configure-build.patch
Patch5: %name-0.3.19cnc53-alt-strsignal.patch
Patch6: %name-0.3.19cnc55-alt-genbasedir.patch
Patch7: %name-0.3.19cnc55-alt-apt-pkg-rpmpm-execute_rpm.patch
Patch8: %name-0.3.19cnc53-stelian-apt-pkg-algorithms-scores.patch
Patch9: %name-0.3.19cnc55-alt-enable-rsh-method.patch

Requires: lib%name = %version-%release, mktemp >= 1:1.3.1, getopt
Requires: %{get_dep rpm}, gnupg, apt-conf

BuildPreReq: librpm-devel >= 4.0.4, rpm-build >= 4.0.4

# Automatically added by buildreq on Tue Mar 26 2002
BuildRequires: bison bzlib-devel gcc-c++ libbeecrypt libdb4 libpopt-devel librpm-devel libstdc++-devel openjade perl-SGMLSpm zlib-devel

%description
A port of Debian's APT tools for RPM based distributions,
or at least for Conectiva. It provides the %name-get utility that
provides a simpler, safer way to install and upgrade packages.
APT features complete installation ordering, multiple source
capability and several other unique features.

This package is still under development.

%define risk_usage ƒанный пакет пока еще находитс€ в стадии разработки.

%description -l ru_RU.CP1251
ѕеренесенные из Debian средства управлени€ пакетами APT, включающие
в себ€ поддержку RPM, выполненную компанией Conectiva (Ѕразили€).
Ётот пакет содержит утилиту %name-get дл€ простой и надежной установки
и обновлени€ пакетов. APT умеет автоматически разрешать зависимости
при установке, обеспечивает установку из нескольких источников и
целый р€д других уникальных возможностей.

%risk_usage

%package -n lib%name
Summary: APT's lib%name-pkg
Group: System/Libraries
Conflicts: %name < %version-%release

%package -n lib%name-devel
Summary: Development files and documentation for APT's lib%name-pkg
Summary(ru_RU.CP1251): ‘айлы и документаци€ дл€ разработчиков, использующих lib%name-pkg
Group: Development/C
Requires: lib%name = %version-%release
Provides: %name-devel = %version
Obsoletes: %name-devel lib%name-pkg-devel lib%name-pkg-doc

%package -n lib%name-devel-static
Summary: Development static library for APT's lib%name-pkg
Summary(ru_RU.CP1251): —татическа€ библиотека APT дл€ разработчиков, использующих lib%name-pkg
Group: Development/C
Requires: lib%name-devel = %version-%release
Provides: %name-devel-static = %version
Obsoletes: %name-devel-static

%description -n lib%name
This package contains APT's lib%name-pkg package manipulation library,
modified for RPM.

This package is still under development.

%description -n lib%name-devel
This package contains the header files and libraries for developing with
APT's lib%name-pkg package manipulation library, modified for RPM.

This package is still under development.

%description -n lib%name-devel-static
This package contains static libraries for developing with APT's
lib%name-pkg package manipulation library, modified for RPM.

This package is still under development.

%description -n lib%name -l ru_RU.CP1251
¬ этом пакете находитс€ lib%name-pkg -- библиотека управлени€ пакетами
из комплекта APT. ¬ отличие от оригинальной версии дл€ Debian, этот
пакет содержит поддержку дл€ формата RPM.

%risk_usage

%description -n lib%name-devel -l ru_RU.CP1251
¬ этом пакете наход€тс€ заголовочные файлы и библиотеки дл€ разработки
программ, использующих lib%name-pkg -- библиотеку  управлени€ пакетами
из комплекта APT. ¬ отличие от оригинальной версии дл€ Debian, этот
пакет содержит поддержку дл€ формата RPM.

%risk_usage

%description -n lib%name-devel-static -l ru_RU.CP1251
¬ этом пакете наход€тс€ статические библиотеки дл€ разработки программ,
использующих lib%name-pkg -- библиотеку  управлени€ пакетами из
комплекта APT. ¬ отличие от оригинальной версии дл€ Debian, этот пакет
содержит поддержку дл€ формата RPM.

%risk_usage

%prep
%setup -q
%patch1 -p1
%patch2 -p1
%patch3 -p1
%patch4 -p1
%patch5 -p1
%patch6 -p1
%patch7 -p1
%patch8 -p1
%patch9 -p1

#install -p -m644 %SOURCE3 po/ru.po

%build
%add_optflags -fno-exceptions -D_GNU_SOURCE

libtoolize --copy --force
aclocal -I buildlib
autoconf

# BEGIN HACK: fix broken files.
%configure --with-proc-multiply=1 --with-procs=%__nprocs
for n in LOCALEDIR `sed -ne 's/.*}\(ENABLE\|HAVE\|NEED\)\(_[A-Z_0-9]*\)\$.*/\1\2/pg' config.status |LC_COLLATE=C sort -u`; do
	%__grep -Fqs "$n" buildlib/config.h.in || echo "#undef $n"
done >config.h.add
cat config.h.add >>buildlib/config.h.in
# END HACK

%configure --with-proc-multiply=1 --with-procs=%__nprocs

find -type f -print0 |
	xargs -r0 %__grep -EZl '/var(/lib)?/state/%name' |
	xargs -r0 %__perl -pi -e 's,/var(/lib)?/state/%name,%_localstatedir/%name,g'
for f in `find -type f -name '*.[58]'`; do
	[ -f "$f.yo" ] || touch -r "$f" "$f.yo"
done
%make clean

%make_build STATICLIBS=1 NOISY=1
tar xzf docs.tar.gz
bzip2 -9 docs/*.text

%install
mkdir -p $RPM_BUILD_ROOT{%_bindir,%_libdir/%name,%_mandir/man{5,8},%_includedir/%name-pkg,%_sysconfdir/%name}
mkdir -p $RPM_BUILD_ROOT%_localstatedir/%name/lists/partial
mkdir -p $RPM_BUILD_ROOT/var/cache/%name/{archives/partial,genpkglist,gensrclist}

cp -a bin/lib*.so* $RPM_BUILD_ROOT%_libdir
cp -a bin/lib*.a* $RPM_BUILD_ROOT%_libdir

install -p -m755 bin/methods/* $RPM_BUILD_ROOT%_libdir/%name
install -p -m755 bin/{%name-*,*list} tools/genbasedir $RPM_BUILD_ROOT%_bindir

install -p -m644 %name-pkg/{,*/}*.h $RPM_BUILD_ROOT%_includedir/%name-pkg

install -p -m644 doc/*.5 $RPM_BUILD_ROOT%_man5dir/
install -p -m644 doc/*.8 $RPM_BUILD_ROOT%_man8dir/

install -p -m644 %SOURCE1 %SOURCE2 $RPM_BUILD_ROOT%_sysconfdir/%name

# Make possible SSH method via RSH one.
%__ln_s rsh $RPM_BUILD_ROOT%_libdir/%name/ssh

%make_install install -C po DESTDIR=$RPM_BUILD_ROOT
%find_lang %name

%post -n lib%name -p /sbin/ldconfig
%postun -n lib%name -p /sbin/ldconfig

%files -f %name.lang
%_bindir/*
%_libdir/%name
%dir %_sysconfdir/%name
%config(noreplace) %_sysconfdir/%name/%name.conf
%config(noreplace) %_sysconfdir/%name/rpmpriorities
%_mandir/man?/*
%doc README* TODO docs/examples REPOSITORIO-APT-HOWTO

%defattr(2770,root,rpm,2770)
%_localstatedir/%name
%_cachedir/%name

%files -n lib%name
%_libdir/*.so.*

%files -n lib%name-devel
%_libdir/*.so
%_includedir/*
%doc docs/*.{text.*,html}

%files -n lib%name-devel-static
%_libdir/*.a

%changelog
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

