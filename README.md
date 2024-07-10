# Shazam Reimplementation

A Shazam reimplementation made for the Advanced Data Structures course @utec

## Table of contents

<!--toc:start-->

- [Shazam Reimplementation](#shazam-reimplementation)
  - [Table of contents](#table-of-contents)
  - [Installation](#installation)
    - [Dependencies](#dependencies)
    - [Running](#running)
  - [Contributing](#contributing)
    - [Pre-commit hooks](#pre-commit-hooks)
  - [Objectives](#objectives)
    - [Main](#main)
    - [Secondary](#secondary)
  - [Results](#results)
  - [Conclusions](#conclusions)
  - [Authors](#authors)
  - [Bibliography](#bibliography)
  <!--toc:end-->

## Installation

**_Windows is not supported by default_**

### Dependencies

- [spdlog](https://github.com/gabime/spdlog)
- [CrowCpp](https://github.com/CrowCpp/Crow)
- [libsndfile](https://github.com/libsndfile/libsndfile)
- [libpqxx](https://github.com/jtv/libpqxx) (probably, not yet implemented) <!--TODO: revisit after db connection-->

### Running

```
cmake -S . -B build
cmake --build build
```

## Contributing

### Pre-commit hooks

These tools are used as part of the pre-commit tests, they can be disabled by editing `.pre-commit-config.yaml` (or `CMakeLists.txt` in the case of [IWYU](https://github.com/include-what-you-use/include-what-you-use?tab=readme-ov-file#using-with-cmake))

- clang-format
- clang-tidy
- cppcheck
- cpplint
- include-what-you-use
- commitizen
  commit message standardizer

## Objectives

### Main

### Secondary

- 1.1
- 1.2

## Results

## Conclusions

- C1
- C2
- C3

## Authors

<table>
    <tr >
        <th style="width:24%;">Juan Diego Prochazca</th>
        <th style="width:24%;">Claudia Noche</th>
        <th style="width:24%;">Enrique Flores</th>
        <th style="width:24%;">Luis Jauregui</th>
    </tr>
    <tr >
        <td><a href="https://github.com/AaronCS25"><img src="https://avatars.githubusercontent.com/u/102536323?s=400&v=4"></a></td>
        <td><a href="https://github.com/Enriquefft"><img src="https://avatars.githubusercontent.com/u/60308719?v=4"></a></td>
        <td><a href="https://github.com/AaronCS25"><img src="https://avatars.githubusercontent.com/u/102536323?s=400&v=4"></a></td>
        <td><a href="https://github.com/Enriquefft"><img src="https://avatars.githubusercontent.com/u/60308719?v=4"></a></td>
    </tr>
</table>

## Algunas cosas

El flujo de este programa sigue de cerca el descrito en el paper de shazam. Sin embargo el paper no explica muchos de los metodos que se deben usar para conseguir los propios datos, solo las cosas que se deben conseguir.

En vista de esto, hemos tenido que tomar muchas decisiones al implementar. Enrique puede hablarles de las que se estan tomando al implementar la STFT. Yo hare un intento de explicar las demas.

En resumen, la idea es la siguiente: 
  toma una cancion (que es una secuencia de muestras tomadas a intervalos regulares y que representan la intensidad del sonido en un momento)

  Calcula algo llamado Espectrograma: una 'foto' que indica la intensidad de distintas frecuencias a lo largo de la cancion. Piensa en esas frecuencias como 'notas'. Ten en mente que esta es una foto, no una lista de notas: ese es el siguiente paso.

  De el Espectrograma, saca los sitios mas intensos. Aca podemos pensar en marcar con una x las notas que mas fuerte suenan. Es bastante intuitivo pensar en esto como si fuera una especie de pentagrama.

  Una ves tienes eso, piensa en la relacion entre una nota y las otras: cuanto mas al frente esta la nota a de la nota b? Cuanto mas aguda o grave es? Nota que por mas que yo mueva cuando aparecen las notas, si las muevo a todas la misma cantidad entonces esta relacion es siempre la misma. De estas relaciones, sacamos hashes para la cancion.

  Hacemos esto para todas las canciones que queremos indexar, y metemos todo esto en un hash. Luego, cuando nos dan un punto de query hasheamos igual ese punto y encontramos todos los hashes del punto que tienen un equivalente en el hashtable global. Evidentemente la cancion mas parecida tendra mas colisiones.

  Pero quiza las notas de las canciones estan todas revueltas! AkA, no siguen la misma secuencia. Por lo tanto tenemos que, para cada hash colisionado, graficar el tiempo en el original vs el tiempo en el query, y ver si se forma una recta. Los puntos que pertenecen a esa recta son los que nos dan por seguro que son iguales en ambos audios. Usamos eso como score. Y listo, podemos hayar canciones por igualdad a un pequenho pedacito!

El espectrograma sale de la stft. De ella puedo decir que se basa en la implementacion de librosa. La fft es una implementacion casi directa de lo que nos dice el cormen.
La extraccion de las notas tiene varias formas de ocurrir. Una es marcar todos los pixeles tales que sean estrictamente mayores a un cuadrado centrado en ellos (GTN) esto se puede hacer en ~O(n^2). Otra es hallando los puntos que son iguales al maximo de un cuadrado centrado en si mismos. La segunda es posible ejecutarla en O(n), pero es vulnerable a marcar multiples puntos vecinos como maximos. La idea de esto salio de https://github.com/lukemcraig/AudioSearch/tree/master. El algoritmo de maxfiltering salio de http://www.code-spot.co.za/2011/01/24/2d-minimum-and-maximum-filters-algorithms-and-implementation-issues/. 

El hash es exactamente el utilizado por el paper: un uint32_t. Su creacion es poco optimizada porque el problema real en tiempos de computo es la STFT y la extraccion de notas!

Decidimos implementar esto en dos ejecutables: un hasheador de los archivos y un programa de queries. El hasheador gurada los resultados en csv porque es mas facil que hacerlo de otra forma.
## Bibliography

- Wang, Avery. (2003). An Industrial Strength Audio Search Algorithm.
