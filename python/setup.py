from setuptools import setup, find_packages

setup(
    name="motionfx",
    version="1.0.0",
    description="Python SDK for MotionFX Animation Engine in OBS Studio",
    author="MotionFX Team",
    packages=find_packages(),
    python_requires=">=3.8",
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
    ],
)
