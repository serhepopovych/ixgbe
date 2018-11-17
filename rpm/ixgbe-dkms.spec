%define module_name ixgbe
%define module_version 5.3.7

%define _srcdir %{_prefix}/src/%{module_name}-%{module_version}
%define _dkmsdir %{_prefix}/lib/dkms

%define _topdir %(pwd)
%define _specdir %{_topdir}
%define _sourcedir %{_topdir}
%define _buildrootdir %{_topdir}
%define _builddir %{_topdir}
%define _rpmdir %{_topdir}/..
%define _srcrpmdir %{_topdir}/..

%define __find_provides  %{_dkmsdir}/find-provides

Summary:	DKMS source for Intel 10GbE Ethernet Controller out of tree driver
Name:		%{module_name}-dkms
Version:	%{module_version}
License:	GPLv2
Packager:	Serhey Popovych <serhe.popovych@gmail.com>
Release:	1%{?dist}
BuildArch:	noarch
Group:		System/Kernel
Requires:	dkms >= 1.95
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root/

%description
This is Intel 10GbE Ethernet Controller out of tree ixgbe driver.

Installation of the ixgbe-dkms package will disable the in-kernel ixgbe
module. To re-enable them, the ixgbe-dkms package must be purged.

This package provides the dkms source code for the ixgbe kernel module.
Kernel source or headers are required to compile these modules.

%prep
if [ "$RPM_BUILD_ROOT" != "/" ]; then
	rm -rf $RPM_BUILD_ROOT
fi

%install
mkdir -p $RPM_BUILD_ROOT/%{_srcdir}
mkdir -p $RPM_BUILD_ROOT/%{_datadir}/%{name}
mkdir -p $RPM_BUILD_ROOT/%{_sysconfdir}/depmod.d
mkdir -p $RPM_BUILD_ROOT/%{_sysconfdir}/modprobe.d

for f in %{_sourcedir}/src/*; do
	install -m 644 ${f} $RPM_BUILD_ROOT/%{_srcdir}/
done

sed -e "s|updates/dkms|extra|g" \
    -e "s|#MODULE_NAME#|%{module_name}|g" \
    -e "s|#MODULE_VERSION#|%{module_version}|g" \
    %{_sourcedir}/dkms/conf > $RPM_BUILD_ROOT/%{_srcdir}/dkms.conf

sed -e "s|updates/dkms|extra|g" \
    %{_sourcedir}/dkms/depmod > $RPM_BUILD_ROOT/%{_sysconfdir}/depmod.d/%{name}.conf

sed -e "s|updates/dkms|extra|g" \
    %{_sourcedir}/dkms/modprobe > $RPM_BUILD_ROOT/%{_sysconfdir}/modprobe.d/%{name}.conf

if [ -f %{_sourcedir}/common.postinst ]; then
	install -m 755 %{_sourcedir}/common.postinst $RPM_BUILD_ROOT/%{_datadir}/%{name}/postinst
fi

%clean
if [ "$RPM_BUILD_ROOT" != "/" ]; then
	rm -rf $RPM_BUILD_ROOT
fi

%post
for POSTINST in %{_dkmsdir}/common.postinst %{_datadir}/%{name}/postinst; do
	if [ -f $POSTINST ]; then
		$POSTINST %{module_name} %{module_version} %{_datadir}/%{name}
		exit $?
	fi
	echo "WARNING: $POSTINST does not exist."
done
echo -e "ERROR: DKMS version is too old and %{name} was not"
echo -e "built with legacy DKMS support."
echo -e "You must either rebuild %{name} with legacy postinst"
echo -e "support or upgrade DKMS to a more current version."
exit 1

%preun
echo -e
echo -e "Uninstall of %{module_name} module (version %{module_version}) beginning:"
dkms remove -m %{module_name} -v %{module_version} --all --rpm_safe_upgrade
exit 0

%files
%defattr(-,root,root)
%config(noreplace) %{_sysconfdir}/depmod.d/%{name}.conf
%config(noreplace) %{_sysconfdir}/modprobe.d/%{name}.conf
%{_srcdir}
%{_datadir}/%{name}
%doc README

%changelog
* %(date "+%a %b %d %Y") %{packager} %{version}-%{release}
- Initial release. (Closes: #123456)
