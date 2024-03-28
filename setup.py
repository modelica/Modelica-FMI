from setuptools import setup

setup(
    name='modelica_fmi',
    version='0.0.0',
    packages=['modelica_fmi'],
    package_data={'modelica_fmi': ['templates/*.mo']},
    install_requires=['fmpy']
)
