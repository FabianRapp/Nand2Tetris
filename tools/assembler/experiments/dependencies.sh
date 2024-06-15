#!/bin/bash

if ! nvidia-smi > /dev/null 2>&1;
then
    echo "NVIDIA CUDA GPU not detected.";
    exit 1;
fi


echo "Checking for required C headers and libraries..."

cat <<EOF > dependencies_test.c
//#include <cuda_runtime.h> //checked later seperatly
#include <x86intrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <pthread.h>
#include <stdatomic.h>
#include <fcntl.h>
#include <unistd.h>
int main()
{

    return 0;
}
EOF

if ! gcc dependencies_test.c -o dependencies_test -lpthread;
then
    echo "Failed to compile the test program. Required C headers or libraries are missing.";
    rm -f dependencies_test.c dependencies_test;
    exit 1;
fi

if ! ./dependencies_test;
then
	rm -f dependencies_test.c dependencies_test;
	exit 1;
fi

if ! nvcc --version > /dev/null 2>&1;
then
	echo "NVIDIA CUDA install not found.";
	if $(gcc --version | grep -i ubuntu);
	then
		echo Ubuntu distro detected, generating cuda install script.
#GENERATE SCRIPT
cat <<'EOF' >install_cuda_ubuntu.sh
#!/bin/bash
recommended_driver=$(sudo ubuntu-drivers devices 2>/dev/null | grep -i nvidia)
driver=$(echo $recommended_driver | grep -m 1 recommended | awk '{
	for (i=1; i<=NF; i++)
		if ($i ~ /nvidia-driver-[0-9]+/)
			print $i
}')
if [ -z "$driver" ];
then
	echo "Error: No recommended NVIDIA driver found."
	exit 1;
fi
sudo apt update && sudo apt upgrade -y
sudo apt install -y "\$driver"
sudo apt install -y nvidia-cuda-toolkit
echo '#----NVIDIA CUDA environment--' >> ~/.bashrc
echo 'export PATH=/usr/local/cuda-12.4/bin:\${PATH:+:\$PATH}' >> ~/.bashrc
echo 'export LD_LIBRARY_PATH=/usr/local/cuda-12.4/lib64\${LD_LIBRARY_PATH:+:\$LD_LIBRARY_PATH}' >> ~/.bashrc
echo '#---------------------------' >> ~/.bashrc
echo 'NVIDIA CUDA toolkit installed, reboot suggested.'
echo 'Test "nvcc --version" after the reboot.'
EOF
#end of cat
			chmod u=rwx install_cuda_ubuntu.sh;
			echo 'To run automated setup run "sudo ./install_cuda_ubuntu.sh"';
		fi #end of ubuntu block
	echo "Official nvida cuda download:"
	#if system does not run ubuntu or the user wants to install cuda diffrently
	echo "https://developer.nvidia.com/cuda-downloads";
	exit 1;
fi

echo "All checks passed.";
rm -f dependencies_test.c dependencies_test;

exit 0;

