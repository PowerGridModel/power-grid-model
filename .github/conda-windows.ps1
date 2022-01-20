# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

conda create --yes -p C:\conda_envs\cpp_pkgs -c conda-forge python=3.10
conda activate C:\conda_envs\cpp_pkgs
conda install --yes -c conda-forge boost-cpp eigen nlohmann_json mkl mkl-devel mkl-include catch2

conda create --yes -p C:\conda_envs\cp38-cp38 -c conda-forge python=3.8
conda activate C:\conda_envs\cp38-cp38
pip install --no-cache-dir -r dev-requirements.txt

conda create --yes -p C:\conda_envs\cp39-cp39 -c conda-forge python=3.9
conda activate C:\conda_envs\cp39-cp39
pip install --no-cache-dir -r dev-requirements.txt

conda create --yes -p C:\conda_envs\cp310-cp310 -c conda-forge python=3.10
conda activate C:\conda_envs\cp310-cp310
pip install --no-cache-dir -r dev-requirements.txt

conda clean --yes --all
