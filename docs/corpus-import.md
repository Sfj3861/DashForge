# Importar candidatos para el corpus de DashForge

El objetivo de investigación es una matriz de **1.820 plazas**: 13 épocas (1.0–1.9, 2.0, 2.1 y 2.2) × 14 cohortes analíticas × 10 plazas.

## Fuente de metadatos

El conjunto público [Geometry Dash Level Dataset](https://huggingface.co/datasets/yusp48/geometry-dash-levels) incluye una exportación de metadatos de unos 53.000 niveles rated/featured, con nombres, IDs, dificultad, estrellas, descargas y versión del editor. La variante `2026_50k_rated_nodata` no contiene `level_string`.

## Ejecución

1. Descarga el archivo `2026_50k_rated_nodata.jsonl` desde la página del conjunto de datos.
2. Instala Python 3.10+ y `openpyxl`.
3. Ejecuta:

   ```bash
   python -m pip install openpyxl
   python tools/populate_corpus.py --input 2026_50k_rated_nodata.jsonl --workbook DashForge_Corpus_1820_plazas.xlsx
   ```

4. Revisa la hoja `Importación metadatos` y cada candidato nuevo en `Matriz 1820`.

## Límites que no se deben ocultar

- La versión del editor no demuestra por sí sola la fecha en que un nivel se publicó o recibió rate. La época asignada por el importador es **inferida** y debe verificarse.
- Los metadatos no contienen la cadena nativa del nivel. No sirven por sí solos para aprender colocación de objetos; hay que recuperar y conservar los datos nativos de cada nivel.
- Los ratings pueden cambiar o desaparecer. Confirma la dificultad vigente y el estado de rate antes de incluir el nivel en el corpus final.
- Las 14 cohortes son una estructura analítica; no todas existían en todas las épocas. Las cohortes de plataforma son agrupaciones de investigación, no iconos clásicos de dificultad independientes.
- El importador solo rellena plazas vacías cuando encuentra una coincidencia de época/dificultad en los metadatos. **No inventa nombres para cubrir huecos** y no certifica que el corpus esté completo.
- Los sustitutos de Gauntlet deben añadirse con su época real y una marca explícita de sustitución; no deben presentarse como niveles históricos de una época anterior.
