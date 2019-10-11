Summary: CursATE log analysis tool
Name: pas-cursate
Version: %{getenv:VERSION}
Release: %{getenv:RELEASE}
License: Qiagen License
URL: https://www.qiagen.com
Group: NGS
Requires: /usr/bin/docker
Requires: bash
Requires: pigz pv
Requires: nc

%description
Cursate allows human friendly processing for PAS logs.

%define package_tag %{getenv:VERSION}-%{getenv:DOCKER_TAG}-%{getenv:RELEASE}

%prep

mkdir -p "${RPM_BUILD_ROOT}/opt/qiagen/cursate/packages"
mkdir -p "${RPM_BUILD_ROOT}/opt/qiagen/cursate/bin"
mkdir -p "${RPM_BUILD_ROOT}/usr/local/bin"
cp "%{getenv:REPO_ROOT}/scripts/cursate" "${RPM_BUILD_ROOT}/opt/qiagen/cursate/bin"

cat > "${RPM_BUILD_ROOT}/usr/local/bin/cursate" <<EOF
#!/bin/bash
exec /opt/qiagen/cursate/bin/cursate --tag "%{package_tag}" "\${@}"
EOF

docker tag cursate:%{getenv:DOCKER_TAG}  cursate:%{package_tag}
docker save cursate:%{package_tag} | pigz > "${RPM_BUILD_ROOT}/opt/qiagen/cursate/packages/cursate.tar.gz"


%files

%dir %attr(0755, root, root) /opt/qiagen/cursate/packages
%attr(0744, root, root) /opt/qiagen/cursate/packages/cursate.tar.gz

%dir %attr(0755, root, root) /opt/qiagen/cursate/bin
%attr(0744, root, root) /opt/qiagen/cursate/bin/cursate

%attr(0744, root, root) /usr/local/bin/cursate


%post

pigz -d -c /opt/qiagen/cursate/packages/cursate.tar.gz | docker load
rm /opt/qiagen/cursate/packages/cursate.tar.gz

%postun

docker rmi cursate:%{package_tag}
docker image prune -f


%clean

docker rmi cursate:%{package_tag}
