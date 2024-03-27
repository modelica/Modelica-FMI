from setuptools import setup

setup(
    name='modelica_fmi',
    packages=['modelica_fmi'],
    package_data={'modelica_fmi': ['templates/*.mo']},
    install_requires=['fmpy']
)
